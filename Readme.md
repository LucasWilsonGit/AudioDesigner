
# AudioDesigner  
Robust DSP chain framework with multiple premade effects and basic miniaudio based IO.

## Run Locally  

Requires a CMake version compatible with FetchDependency and  some other modern CMake features. Recommended CMake 3.15

It may be necessary to specify -DWIN32 as a flag when building for windows platforms, I've not yet fully moved across to checking CMAKE_PLATFORM_NAME instead of using the deprecated platform variables.

## Contributing  

Please don't.

## License  

Licenses for dependencies will be inside the dependency repos when they are fetched. A jumbo SLA will be generated in the CMake build tree for your convenience. Inability to source a license for any dependencies will cause the build to fail.

This repo is private. Do not take, copy, use or modify this code. 