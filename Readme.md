
# AudioDesigner  
Robust DSP chain framework with multiple premade effects and basic miniaudio based IO.

## Run Locally  

Requires a CMake version compatible with FetchDependency and  some other modern CMake features. Recommended CMake 3.15

It may be necessary to specify -DWIN32 as a flag when building for windows platforms, I've not yet fully moved across to checking CMAKE_PLATFORM_NAME instead of using the deprecated platform variables.

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
10. Restart the AudioEngineAllocator service.

## Contributing  

Please don't.

## License  

Licenses for dependencies will be inside the dependency repos when they are fetched. A jumbo SLA will be generated in the CMake build tree for your convenience. Inability to source a license for any dependencies will cause the build to fail.

This repo is private. Do not take, copy, use or modify this code. 