# RTIK

Copyright (c) 2017 Henry Cooney

RTIK (Real Time Inverse Kinematics) is an inverse kinematics system for Unreal Engine 4. UE4 includes several inverse kinematics solvers; however, these are only the basic building blocks of a functional IK system. Substantial additional work is required to make IK look good.


This project attempts to create a modular, easy-to-use IK system for UE4. The goals are as follows:

 - Implement 'full-body' IK: that is, the ability to smoothly IK the hands and feet of a humanoid character, with appropriate adjusment to the torso, etc. See [1] for an idea of how this is done (though my is slightly different).

 - Provide a modular set of AnimGraph nodes that can be used to build IK setups. These nodes are 'lower level', but should enable animators to create custom IK setups without needing to write C++.

 - Create higher-level AnimGraph nodes for more common IK setups. These should make common IK scenarios easy to set up. In particular, a very easy-to-use setup for humanoids will be provided.

 - Add tooling support; that is, a graphical editor for setting up IK and testing with different animations (this is a longer-term goal)

 - Implement 'prone' IK: that is, the ability for characters to lay down or crawl with minimal clipping through uneven ground (this is a longer-term goal).

## How To Use

   RTIK is currently distributed as a UE4 non-game module. Source code can be found in the Plugins/ directory. Installation instructions will be posted soon. RTIK has been tested with UE4 4.16 only, although it may be work with other engine versions.

   This repo contains an example project, consiting of a demo area, and a character with IK applied. You can experiment with the demo scene, or check the included AnimBPs for an example of using RTIK.

   If you are interested in integrating RTIK with an existing project, you should download the Plugins/ directory only.

## Status

Updated 8/15/2017 

RTIK has been repackaged as a non-game module. This should make it easier to integrate with other Unreal projects.

Leg / Foot IK is working. I've tested it pretty extensively and it looks great!

Upper body IK is also working, though there is no lower-hip movement.

Closed loop and noisy-three-point solvers have been implemented. However, before adding new features, I'd like to clean up what I already have.

So, current tasks are:
    - Repackage as a non-game module
    - Improve ease-of-use
    - Make some early 'docs' and demo videos!

Upper body IK is also working, with nice upper body / torso rotations. I haven't implemented any kind of arm constraints or joint
corrections yet; to me, this is less important than with the legs since a. the targets are not procedural and b. Unreal's two-bone
IK nodes actually work pretty well for arm IK; you don't usually get bad arm positions unless you do something stupid.

I haven't attempted to integrate hip / lower body movements with upper body IK. Basically, clever use of FABRIK closed-loop
solvers on the upper / lower body triangles allow the character to bend over in a more realistic way, moving the hips back / down
instead of bending at the waist only. I don't think many people would actually use this, but it would look cool.

I have done some performance testing. Performance is good; full body IK costs less than 200us / character on my laptop.

## License

   This project is made available under the MIT license. See the file `LICENSE` in this distribution for license terms.

## References

  [1] A Aristidou, Y Chrysanthou, J Lasenby, "Extending FABRIK with model constraints", Computer Animation and Virtual Worlds, vol. 27, no. 1, pp.35-57, Jan. 2016
