# RTIK

Copyright (c) 2017 Henry Cooney

RTIK (Real Time Inverse Kinematics) is an inverse kinematics system for Unreal Engine 4. UE4 includes several inverse kinematics solvers; however, these are only the basic building blocks of a functional IK system. Substantial additional work is required to make IK look good.

A video demo can be found here: [demo](https://youtu.be/Cm-hjahqLh8)

![Alt text](media/lefthand.gif?raw=true "Hand IK")

This project attempts to create a modular, easy-to-use IK system for UE4. The goals are as follows:

 - Implement 'full-body' IK: that is, the ability to smoothly IK the hands and feet of a humanoid character, with appropriate adjusment to the torso, etc. See [1] for an idea of how this is done (though my approach is slightly different).

 - Provide a modular set of AnimGraph nodes that can be used to build IK setups. These nodes are 'lower level', but should enable animators to create custom IK setups without needing to write C++.

 - Create higher-level AnimGraph nodes for more common IK setups. These should make common IK scenarios easy to set up. In particular, a very easy-to-use setup for humanoids will be provided.

 - Add tooling support; that is, a graphical editor for setting up IK and testing with different animations (this is a longer-term goal)

 - Implement 'prone' IK: that is, the ability for characters to lay down or crawl with minimal clipping through uneven ground (this is a longer-term goal).

## Who it's for

   RTIK is primarily aimed at UE4 developers who want an extra level of animation quality. Currently, it provides nice-looking full-body IK for humanoid characters; its modular design may allow you to create custom IK setups, as well. 

   RTIK may also be useful for animation or motion researchers, who need an IK layer underneath other procedural animation systems.

## How To Use

   RTIK is currently distributed as a UE4 non-game module. Source code can be found in the Plugins/ directory.
   
   This repo contains an example project, consiting of a demo area, and a character with IK applied. You can experiment with the demo scene, or check the included AnimBPs for an example of using RTIK. See 'Stand-Alone Demo Installation' for details of using the standalone RTIK demo.
   
   The demo character has locomotion IK applied to the feet (arm IK is disabled by default). You can manually IK its arms and legs by selecting the character, going to the 'IK' tab under the details panel, and setting a limb to IK onto a world target.
   
   If you are interested in integrating RTIK with an existing project, you will need the contents of the Plugins/ directory only. See 'Installing RTIK as a Plugin" for more instructions.

## Requirements

   RTIK has been tested under Windows 10 only. It has been tested with Unreal Engine 4.16, but may work with other engine versions. UE4 is available at https://www.epicgames.com/
   
   Building RTIK requires MS Visual Studio. MSVS may be found at https://www.visualstudio.com/

## Stand-Alone Demo Installation
   
   NOTE -- If you are having trouble building the project, or the IKDemo.sln doesn't load properly, try doing the following:
    
   - Delete the Binaries/ and Intermediate/ directories
   - Delete Plugins/rtik/Binaries and Plugins/rtik/Intermediate
   - Right click IKDemo.uproject and 'Generate Project Files'
   - Try building again
   
   Unfortunately the build process has been flaky since I turned the project into a plugin. I'm working on it! 
   
   RTIK may be run as a stand-alone UE4 project. To run, you must download UE 4.16, and MS Visual Studio. Steps to run follow:

   - Clone this repo
   - Right click on IKDemo.uproject and select 'Generate project files'
   - Double click on IKDemo.uproject to start (click 'Yes' to build if prompted)
   	
   This should start the demo in-editor. RTIK has been applied to CHAR_PatrolMannequin in the starting scene. You can select IK target actors, and enable/disable targeted IK, in CHAR_PatrolMannequin's details panel, under IK.

   An example AnimBP can be found at Content/Blueprints/Characters/PatrolMannequin/ANIMBP_IK_PatrolMannequin.

## Installing RTIK as a Plugin

   To install as a plugin with an existing project:

   - Place the contents of the Plugins/ directory in your projects Plugins/ directory
   - Start your project. If prompted to rebuild, click 'Yes'.
   - Ensure that the plugin is enabled by going to the Edit/Plugins menu. In the Plugins window, find Real Time Inverse Kinematics under the Project/Other tab, and make sure the 'Enabled' box is checked.

   Note that you MUST have C++ in your project to install RTIK, otherwise it cannot be compiled, as your project will have no .sln file to build from. If your project is blueprint-only, simply add a single C++ class by clicking 'Add New' in the Content brower, and selecting 'New C++ Class'. Creating any C++ class will generate the necessary project files.

## Status

Updated 8/16/2017 

An early build of RTIK is now available as a non-game module. It provides full body IK for humanoids. While the current version of RTIK is very much an early alpha, you are welcome to experiment with it!

## Contact Info

For questions, concerns, comments, et cetera please contact me (Henry) at hacoo36@gmail.com. 

## License

   This project is made available under the MIT license. See the file `LICENSE` in this distribution for license terms.

## References

  [1] A Aristidou, Y Chrysanthou, J Lasenby, "Extending FABRIK with model constraints", Computer Animation and Virtual Worlds, vol. 27, no. 1, pp.35-57, Jan. 2016
