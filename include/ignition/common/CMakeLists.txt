if(SKIP_DEPRECATED)
  set(EXCLUDE_FILES
    Plugin.hh
    PluginInfo.hh
    PluginLoader.hh
    PluginMacros.hh
    PluginPtr.hh
    SpecializedPlugin.hh
    SpecializedPluginPtr.hh
    TemplateHelpers.hh
    detail/Plugin.hh
    detail/PluginLoader.hh
    detail/PluginMacros.hh
    detail/PluginPtr.hh
    detail/SpecializedPlugin.hh
    detail/TemplateHelpers.hh
  )
else()
  set(EXCLUDE_FILES)
endif()

ign_install_all_headers(EXCLUDE_FILES ${EXCLUDE_FILES})
