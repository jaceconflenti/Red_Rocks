# Red Rocks Amphitheatre 
## Steven Jace Conflenti
### CSCI 4229 Fall 16

## Run with make clean; make; ./redrocks 

## The best way to view the program is to let the camera cycle on it's own (every four seconds). You can also stop the automatic camera movement with the '`' (tilde) key and view the scene manually. Click and drag with the left mouse button to move in the X-Z plane, or click and drag with the right mouse button to move in the X-Y plane. You can use the arrow keys to further rotate the camera; however, note that the left and right keys are sensitive.

### Key bindings:
 * ESC		Exit
 * Spacebar Pause music
 * n/m  	Previous/Next song
 * a        Toggle performance mode
 * s 		Toggle sky
 * d 		Toggle light movement
 * f 		Cycle light intensity
 * [/] 		Move light
 * -/+ 		Change light elevation
 * `    	Toggles automatic camera cycling
 * 1		Viewing angle 1
 * 2 		Viewing angle 2
 * 3 		Viewing angle 3
 * 4 		Viewing angle 4
 * 5 		Viewing angle 5
 * 6 		Viewing angle 6
 * 7/8		Decrease/Increase fov
 * 9/0 		Decrease/Increase dim

### Things I Spent Lots of Time On
* Skydome - I spent way too much time trying to create a far too complex sky. Eventually I simplified it, but even the simplified version took me a long time to figure out.
* Stands - I created all elements of the stands by hand. They are very detailed (curved, complex, textured, etc.)
* Rock formations - Modeled them by hand in blender.
* Performance issues - Now the only thing that really saps performance are the trees (also have performance mode).
* Hand creating other objects - Almost everything you see in the scene (besides the spotlights and trees) I created by hand.
* Converting to and getting things working with the right version of SDL - luckily I did this relatively early on or it really would have been painful.