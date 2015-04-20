#include <Hadouken/Scripting/Modules/BitTorrent/TorrentHandleWrapper.hpp>

#include <Hadouken/BitTorrent/PeerInfo.hpp>
#include <Hadouken/BitTorrent/TorrentInfo.hpp>
#include <Hadouken/BitTorrent/TorrentHandle.hpp>
#include <Hadouken/BitTorrent/TorrentStatus.hpp>
#include <Hadouken/Scripting/Modules/BitTorrent/PeerInfoWrapper.hpp>
#include <Hadouken/Scripting/Modules/BitTorrent/TorrentInfoWrapper.hpp>
#include <Hadouken/Scripting/Modules/BitTorrent/TorrentStatusWrapper.hpp>

#include "../common.hpp"
#include "../../duktape.h"

using namespace Hadouken::BitTorrent;
using namespace Hadouken::Scripting::Modules;
using namespace Hadouken::Scripting::Modules::BitTorrent;

const char* TorrentHandleWrapper::field = "\xff" "TorrentHandle";

void TorrentHandleWrapper::initialize(duk_context* ctx, std::shared_ptr<Hadouken::BitTorrent::TorrentHandle> handle)
{
    duk_function_list_entry handleFunctions[] =
    {
        { "getPeers", TorrentHandleWrapper::getPeers, 0 },
        { "getStatus", TorrentHandleWrapper::getStatus, 0 },
        { "getTorrentInfo", TorrentHandleWrapper::getTorrentInfo, 0 },
        { "moveStorage", TorrentHandleWrapper::moveStorage, 1 },
        { "pause", TorrentHandleWrapper::pause, 0 },
        { "resume", TorrentHandleWrapper::resume, 0 },
        { NULL, NULL, 0 }
    };

    duk_idx_t handleIndex = duk_push_object(ctx);
    duk_put_function_list(ctx, handleIndex, handleFunctions);

    duk_push_pointer(ctx, new TorrentHandle(*handle));
    duk_put_prop_string(ctx, handleIndex, field);

    DUK_READONLY_PROPERTY(ctx, handleIndex, infoHash, TorrentHandleWrapper::getInfoHash);
    DUK_READONLY_PROPERTY(ctx, handleIndex, queuePosition, TorrentHandleWrapper::getQueuePosition);
    DUK_READONLY_PROPERTY(ctx, handleIndex, tags, TorrentHandleWrapper::getTags);

    duk_push_c_function(ctx, TorrentHandleWrapper::finalize, 1);
    duk_set_finalizer(ctx, -2);
}

duk_ret_t TorrentHandleWrapper::finalize(duk_context* ctx)
{
    if (duk_get_prop_string(ctx, -1, field))
    {
        void* ptr = duk_get_pointer(ctx, -1);
        TorrentHandle* handle = static_cast<TorrentHandle*>(ptr);
        delete handle;
    }

    return 0;
}

duk_ret_t TorrentHandleWrapper::getInfoHash(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    duk_push_string(ctx, handle->getInfoHash().c_str());
    return 1;
}

duk_ret_t TorrentHandleWrapper::getPeers(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);

    int arrayIndex = duk_push_array(ctx);
    int i = 0;

    for (PeerInfo peer : handle->getPeers())
    {
        PeerInfoWrapper::initialize(ctx, peer);
        duk_put_prop_index(ctx, arrayIndex, i);

        ++i;
    }

    return 1;
}

duk_ret_t TorrentHandleWrapper::getQueuePosition(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    duk_push_int(ctx, handle->getQueuePosition());
    return 1;
}

duk_ret_t TorrentHandleWrapper::getStatus(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    TorrentStatusWrapper::initialize(ctx, handle->getStatus());
    return 1;
}

duk_ret_t TorrentHandleWrapper::getTags(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);

    int arrayIndex = duk_push_array(ctx);
    int i = 0;

    for (std::string tag : handle->getTags())
    {
        duk_push_string(ctx, tag.c_str());
        duk_put_prop_index(ctx, arrayIndex, i);

        ++i;
    }

    return 1;
}

duk_ret_t TorrentHandleWrapper::getTorrentInfo(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    std::unique_ptr<TorrentInfo> info = handle->getTorrentFile();

    if (info)
    {
        TorrentInfoWrapper::initialize(ctx, *handle, std::move(info));
    }
    else
    {
        duk_push_null(ctx);
    }

    return 1;
}

duk_ret_t TorrentHandleWrapper::moveStorage(duk_context* ctx)
{
    std::string path(duk_require_string(ctx, 0));

    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    handle->moveStorage(path);

    return 0;
}

duk_ret_t TorrentHandleWrapper::pause(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    handle->pause();
    return 0;
}

duk_ret_t TorrentHandleWrapper::resume(duk_context* ctx)
{
    TorrentHandle* handle = Common::getPointer<TorrentHandle>(ctx, field);
    handle->pause();
    return 0;
}