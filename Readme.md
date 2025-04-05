
# AudioDesigner  
Microservice style audio processing framework I'm working on.

## How to build and run binaries

Requires at least CMake 3.25 

Currently I have only implemented memory mapping on Windows, need to look into libhugetlbfs
Enabling large pages in the group policy will be necessary
Configuring the project should pull and patch any external dependencies
Currently it is necessary to build with --target-clean. I am still looking into why but it's low priority for me currently since build times are still quick.

dsp_basic expects a conf.cfg file in the cwd. A conf.cfg will be copied into the build dir automatically during configuration, so just running dsp_basic from the build dir should be sufficient.

the binary for dsp_basic can be found in ${CMAKE_BINARY_DIR}/src/dsp_basic/
conf.cfg for dsp_basic can be found in ./src/dsp_basic/

#### How to build

I recommend making a build directory within the project.

~~~
   mkdir ./_build && cd ./_build
   cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build . -j24
~~~

When building on windows with GNU build tools / mingw, any issues with multiple target patterns from compiler-depends.make files is likely a problem from MSYS/MinGW. I have been using mingw-w64-x86_64-cmake and mingw-w64-x86_64-ninja from an MSys64 shell to avoid this problem. 

A workaround if you are not willing to use the same build tools is to use --clean-first to avoid having issues from dependencies, but this will extend build times.

##### MSVC 

Should build pretty trivially with MSVC from the command line invoking CMake, but the configuration must also be passed to MSVC at the build stage:
~~~
   cd ./_build
   cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build . -j24 --config Release
~~~

#### Enabling gpedit.msc on Windows:

Run the following two commands in sequence:
`FOR %F IN ("%SystemRoot%\servicing\Packages\Microsoft-Windows-GroupPolicy-ClientTools-Package~*.mum") DO (DISM /Online /NoRestart /Add-Package:"%F")`

`FOR %F IN ("%SystemRoot%\servicing\Packages\Microsoft-Windows-GroupPolicy-ClientExtensions-Package~*.mum") DO (DISM /Online /NoRestart /Add-Package:"%F")`

#### Enabling Large pages on Windows:

This requires a user account be configured to run the allocator service, which will then map named shared memory with large pages to be used by other AudioDesigner binaries.

1. On the **Start Menu**, select **Run**. In the **Open** box, type `gpedit.msc`. The **Group Policy** dialog box opens.
2. On the **Local Group Group Policy** console, expand **Computer Configuration**.
3. Expand **Windows Settings**.
4. Expand **Security Settings**.
5. Expand **Local Policies**.
6. Select the **User Rights Assignment** folder. The policies will be displayed in the details pane.
7. In the pane, scroll to and double-click the **Lock pages in memory** policy.
8. In the **Local Security Policy** Setting dialog box, select **Add User or Group....** 
   Add all users of AudioEngine that you would like to allow Large Page allocations. Consider making a separate user specifically for this policy, and making this the default user of the AudioEngineAllocator Service.
9. Select OK.

#### Running tests

The project has ctest enabled in the root build tree so you should just be able to run ctest . from within the build directory. 

**When building tests with MSVC stick to the release configuration, otherwise the block allocator unit tests will fail when the VC++ stl implementation starts allocating random crap related to vector implementation state debugging stuff. Maybe there is some way I can set up my flags to stop this but I haven't found it yet.**

#### Debugging

The project is compiled with -O3 for Release and RelWithDebInfo configurations. If you need to debug you can try debugging with RelWithDebInfo but if functions are being inlined aggressively it may be necessary to build a Debug configuration instead which does not have the -O3 flag.

I have not configured CXX flags for MSVC or clang. 

## Contributing  

Fork the repository and create a new branch for your work.
Make changes with clear code comments explaining your approach. Try to follow existing conventions in the code.
Open a PR into main linking any related issues. Provide context on changes.
I will review PRs when possible. Please be patient. Once approved, code will be merged.

## License  

Licenses for dependencies will be inside the dependency repos when they are fetched. A jumbo SLA will be generated in the CMake build tree for your convenience. Inability to source a license for any dependencies will cause the build to fail.

This project is licensed under the polyform strict license.
