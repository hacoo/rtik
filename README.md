# IKMod

Copyright (c) 2017 Henry Cooney

IKMod is a modular inverse kinematics system for Unreal Engine 4. UE4 includes several inverse kinematics solvers; however, these are only the basic building blocks of a functional IK system. Substantial additional work is required to make IK look good.

This project attempts to create a modular, easy-to-use IK system for UE4. The goals are as follows:

 - Implement 'full-body' IK: that is, the ability to smoothly IK the hands and feet of a humanoid character, with appropriate adjusment to the torso, etc. See [1] for an idea of how this is done (though my approach will probably be a little different).

 - Provide a modular set of AnimGraph nodes that can be used to build IK setups. These nodes will be 'lower level', but should enable animators to create custom IK setups without needing to write C++.

 - Create higher-level AnimGraph nodes for more common IK setups. These should make common IK scenarios veryeasy to set up. In particular, a very easy-to-use setup for humanoids will be provided.

 - Add tooling support; that is, a graphical editor for setting up IK and testing with different animations (this is a longer-term goal)

 - Implement 'prone' IK: that is, the ability for characters to lay down or crawl with minimal clipping through uneven ground (this is a longer-term goal).

## How To Use

   More information incoming! IKMod is currently in the early phases and is not useful.

   IKMod is currently distributed as a stand-alone UE4 project, similar to UE4 Content Examples. To run it, you will need Unreal Engine 4.16, available at `https://www.unrealengine.com/`. Once UE4 is installed, simply run ue4ik.uproject to start the project in-editor.

   IK has been applied to several characters in the starting scene. You can look at their animgraphs to see how they have been set up, or run in-editor to see the results.

   Since this is currently a stand-alone project, you will need to integrate the C++ code (in the Source directory) by hand to use it with your project. Later releases will package the project as a module for easy integration.
 
## Status

   This project is under construction. Check back soon!

## License

   This project is made available under the MIT license. See the file `LICENSE` in this distribution for license terms.

## References

   A Aristidou, Y Chrysanthou, J Lasenby, "Extending FABRIK with model constraints", Computer Animation and Virtual Worlds, vol. 27, no. 1, pp.35-57, Jan. 2016