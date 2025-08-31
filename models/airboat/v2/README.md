# V2

This design aims to refine the following issues with v1:

- The front outriggers were too high up.  It made it so that the water rushed over the bow, requiring extra power to get planing, and once acheieved, the boat just took off faster than expected.  This resulted in the first and only crash of v1 on it's maiden voyage.
- Using the slicers method of chopping into 3 parts, while okay, needed a lot of attention to gluing and led to a catastrophic failure of the front third ripping apart from the rest of the hull.  Part was my error in gluing and assembly, however, I theorize that a dovetail connection may be a better solution to the snaps used.
- The gaskets were not tall enough, anough half a millimeter might be good.
- The gaskets are wasteful of space - The screws could go through the gaskets.
- The screws were on the wrong side of the gaskets, leading to a way for water to get in.
- I missed a gasket around the pylon routing when chopping into 3.  I'd like to try to see if I can gasket this, rather than just rely on epoxy.  While the epoxy was fine, it was a bit of a PITA to apply.
- The pylon broke around the base when crashed.  I am guessing this is the inertia from the motor.  Can this be made stronger (i.e. not right angles to the base plate?)
- Control rod was too close to the rudder - make the servo mount horizontal for easier placement.
- Control rod started to bind on left turns.  Can the control horn be moved to the hinge to help reduce binding?
- Need to use parameterization better.

## Optional design features
Let's use the concept of stretch goals to set some other great features.
- The connections between the 3 parts required permanent gluing.  Can I design something that relies on screws or something removable so that in the case of a design error or a crash, then only those pieces can be removed and replaced??  Trying to think more circularly. Eff late stage capitalism, and it's sole focus on profits.
- The rudder, while it worked well, broke upn rescue and placement on the hard surface of my rescue kayak.  Let's redesign something so that if the rudder/skeg breaks one does not need to replace the whole thing.
- The rudder in the PIP edition needs to be printed in a manner that the weakest direction is the direction that takes all the force.  Perhaps a to piece print is better (rudder + beam?)
- Can I do a spring loaded rudder such that if it hits something, it hinges upward and autoreturns (via a spring or elastic), rather than breaks?
- Try printing in PLA, PLA seems less "brittle" than PETG.
- A single design file might be nice (i.e. multiple bodies) so that one can tell if everything aligns and fits well.

## What went well?
- The insert for the internals was nice to allow for experimentation - keep this.
- Once planing, the boat was fast!  Unfortunately due to pilot inexperience/trying to get it to plane, this lasted for about 3.37 seconds!  LOL
- The width of the pontoons seemed good.
- The overall pylon design seems strong.
- Hull height seems okay, barring the initial planing issue.

# Notes
Control Rod - mid hull
radius: 1.2mm
Height: <<Params>>.WallThickness + <<Params>>.SealWidth + <<Params>>.InnerWallThickness + <<Params>>.DoveTailLength
X-pos: <<Params>>.HullWidthHalved - <<Params>>.WallThickness - <<Params>>.SealWidth - <<Params>>.InnerWallThickness - <<Params>>.Tolerance - 20 mm
Y-pos: -(<<Params>>.MidSectionLength / 2 - <<Params>>.WallThickness - <<Params>>.SealWidth - <<Params>>.InnerWallThickness)
Z-Pos: <<Params>>.HullHeight - <<Params>>.PylonBasePlateThickness - <<Params>>.SealChannelHeight - 1.5 mm
Around X-Axis: 90 deg