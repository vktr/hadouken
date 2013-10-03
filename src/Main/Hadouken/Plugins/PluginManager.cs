﻿using System;
using System.Runtime.InteropServices;
using Hadouken.Framework;
using Hadouken.Plugins.Metadata;
using Hadouken.Sandbox;
using Hadouken.IO;
using System.IO;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;
using NLog;

namespace Hadouken.Plugins
{
    public sealed class PluginManager : IPluginManager
    {
        private static readonly Logger Logger = LogManager.GetCurrentClassLogger();
        private static readonly JsonSerializerSettings SerializerSettings = new JsonSerializerSettings();

        private readonly string _path;
        private readonly IManifest _manifest;
        private readonly IFileSystem _fileSystem;
        private readonly IBootConfig _bootConfig;
        private SandboxedEnvironment _sandboxedEnvironment;

        static PluginManager()
        {
            SerializerSettings.Converters.Add(new VersionConverter());
        }

        public PluginManager(string path, IManifest manifest, IFileSystem fileSystem, IBootConfig bootConfig)
        {
            State = PluginState.Unloaded;

            _path = path;
            _manifest = manifest;
            _fileSystem = fileSystem;
            _bootConfig = bootConfig;
        }

        public IManifest Manifest
        {
            get { return _manifest; }
        }

        public PluginState State { get; private set; }

        public void Load()
        {
            Logger.Info("Loading plugin {0}", _manifest.Name);

            State = PluginState.Loading;

            var setupInfo = new AppDomainSetup
                {
                    ApplicationBase = _path
                };

            var assemblyName = typeof (SandboxedEnvironment).Assembly.Location;
            var typeName = typeof (SandboxedEnvironment).FullName;

            var domain = AppDomain.CreateDomain(Guid.NewGuid().ToString(), null, setupInfo);

            Logger.Debug("Creating sandboxed environment");
            _sandboxedEnvironment = (SandboxedEnvironment) domain.CreateInstanceFromAndUnwrap(assemblyName, typeName);
            Logger.Debug("Loading {0} in sandboxed environment", _manifest.Name);
            _sandboxedEnvironment.Load(_bootConfig);

            State = PluginState.Loaded;
        }

        public void Unload()
        {
            State = PluginState.Unloading;

            if (_sandboxedEnvironment == null) return;

            var domain = _sandboxedEnvironment.GetAppDomain();

            if (domain == null) return;

            Logger.Debug("Unloading AppDomain for plugin {0}", _manifest.Name);
            AppDomain.Unload(domain);

            State = PluginState.Unloaded;
        }
    }
}
