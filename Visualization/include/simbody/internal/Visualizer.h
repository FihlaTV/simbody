#ifndef SimTK_SIMBODY_VISUALIZER_H_
#define SimTK_SIMBODY_VISUALIZER_H_

/* -------------------------------------------------------------------------- *
 *                              SimTK Simbody(tm)                             *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK biosimulation toolkit originating from           *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2010 Stanford University and the Authors.           *
 * Authors: Peter Eastman, Michael Sherman                                    *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

/** @file 
Declares the Visualizer class used for collecting Simbody simulation results 
for display and interaction through the VisualizationGUI. **/


#include "simbody/internal/common.h"

#include <utility> // for std::pair

namespace SimTK {

class MultibodySystem;
class DecorationGenerator;

/** Provide simple visualization of and interaction with a Simbody simulation,
with real time control of the frame rate.\ There are several operating modes
available, including real time operation permitting responsive user interaction
with the simulation.

Frames are sent to the renderer at a regular interval that is selectable, with
a default rate of 30 frames/second. The various operating modes provide 
different methods of controlling which simulation frames are selected and how
they are synchronized for display.

<h3>Visualization modes</h3>

There are three operating modes for the Visualizer's display of simulation
results, selectable via setMode():

- <b>PassThrough</b>. This is the default mode. It sends through to the 
renderer \e every frame that is received from the simulation, slowing down the 
simulation if necessary so that the frames are presented at a selected frame 
rate. But note that the simulation time will not be synchronized to real time; 
because Simbody simulations generally proceed at a variable rate, the 
regularly-spaced output frames will represent different amounts of simulated 
time. If you want real time and simulation time synchronized, use the RealTime 
mode.

- <b>Sampling</b>. This mode is useful for monitoring a simulation that is
allowed to run at full speed. We send frames for display at a maximum rate 
given by the frame rate setting. After a frame is sent, all subsequent frames 
received from the simulation are ignored until the frame interval has passed; 
then the next received frame is displayed. This allows the simulation to 
proceed at the fastest rate possible but time will be irregular and not all 
frames generated by the simulation will be shown.

- <b>RealTime</b>. Synchronize frame times with the simulated time, slowing
down the simulation if it is running ahead of real time, as modifed by the
time scale; see setRealTimeScale(). Frames are sent to the renderer at the
selected frame rate. Smoothness is maintained by buffering up frames before 
sending them; interactivity is maintained by keeping the buffer length below 
human perception time (150-200ms). The presence and size of the buffer is 
selectable; see setDesiredBufferLengthInSec().

<h3>User interaction</h3>

The Simbody VisualizationGUI provides some user interaction of its own, for
example allowing the user to control the viewpoint and display options. User
inputs that it does not interpret locally are passed on to the simulation,
and can be intercepted by registering InputListeners with the Visualizer. The
Visualizer provides a class Visualizer::InputSilo which is an InputListener
that simply captures and queues all user input, with the intent that a running
simulation will occasionally stop to poll the InputSilo to process any input
that has been collected. **/
class SimTK_SIMBODY_EXPORT Visualizer {
public:
class FrameController; // defined below
class InputListener;   // defined in Visualizer_InputListener.h
class InputSilo;       //                 "


/** Construct new Visualizer using default window title (the name of the 
current executable). **/
Visualizer(MultibodySystem& system);
/** Construct new Visualizer with a given window title. **/
Visualizer(MultibodySystem& system, const String& title);
/** InputListener, FrameController, and DecorationGenerator objects are 
destroyed here. **/
~Visualizer();

/** These are the operating modes for the Visualizer, with PassThrough the 
default mode. See the documentation for the Visualizer class for more
information about the modes. **/
enum Mode {
    /** Send through to the renderer every frame that is received from the
    simulator (default mode). **/
    PassThrough = 1,
    /** Sample the results from the simulation at fixed real time intervals
    given by the frame rate. **/
    Sampling    = 2,
    /** Synchronize real frame display times with the simulated time. **/
    RealTime    = 3
};

/** These are the types of backgrounds the VisualizationGUI currently supports.
You can choose what type to use programmatically, and users can override that
choice in the GUI. Each of these types may use additional data (such as the
background color) when the type is selected. **/
enum BackgroundType {
    /** Show a ground plane on which shadows may be cast, as well as a sky
    in the far background. **/
    GroundAndSky = 1,
    /** Display a solid background color that has been provided elsewhere. **/
    SolidColor   = 2
};

/** @name               VisualizerGUI display options
These methods provide programmatic control over some of the VisualizerGUI's
display options. Typically these can be overridden by the user directly in
the GUI, but these are useful for setting sensible defaults. In particular,
the Ground and Sky background, which is the GUI default, is not appropriate
for some systems (molecules for example). **/
/**@{**/

/** Change the background mode currently in effect in the GUI.
@param background   the new background type to use **/
void setBackgroundType(BackgroundType background) const;

/** Set the position and orientation of the ground plane.\ This will be used
when the Ground and Sky background mode is in effect.
@param axis     the axis to which the ground plane is perpendicular; + -> up
@param height   the position of the ground plane along the specified axis **/
void setGroundPosition(const CoordinateAxis& axis, Real height);

/** Set the background color.\ This will be used when the solid background
mode is in effect but has no effect otherwise.
@param color        the background color in r,g,b format with 0..1 range **/
void setBackgroundColor(const Vec3& color) const;

/** Control whether shadows are generated when the Ground&Sky background
mode is in effect.
@param showShadows      set true to have shadows generated; false for none **/
void setShowShadows(bool showShadows) const;
/**@}**/

/** @name                  Visualizer options
These methods are used for setting a variety of options for the Visualizer's
behavior, normally prior to sending it the first frame. **/
/**@{**/
/** Set the operating mode for the Visualizer. See \ref Visualizer::Mode for 
choices, and the discussion for the Visualizer class for meanings. **/
void setMode(Mode mode);
/** Get the current mode being used by the Visualizer. See \ref Visualizer::Mode
for the choices, and the discussion for the Visualizer class for meanings. **/
Mode getMode() const;

/** Set the frame rate in frames/sec (of real time) that you want the 
Visualizer to attempt to achieve. This affects all modes. The default is 30 
frames per second for RealTime and Sampling modes; Infinity (that is, as fast 
as possible) for PassThrough mode. Set the frame rate to zero to return 
to the default behavior. **/
void setDesiredFrameRate(Real framesPerSec);
/** Get the current value of the frame rate the Visualizer has been asked to 
attempt; this is not necessarily the rate actually achieved. A return value of 
zero means the Visualizer is using its default frame rate, which may be
dependent on the current operating mode. 
@see setDesiredFrameRate() for more information. **/
Real getDesiredFrameRate() const;

/** In RealTime mode we normally assume that one unit of simulated time should
map to one second of real time; however, in some cases the time units are not 
seconds, and in others you may want to run at some multiple or fraction of 
real time. Here you can say how much simulated time should equal one second of
real time. For example, if your simulation runs in seconds, but you want to 
run twice as fast as real time, then call setRealTimeScale(2.0), meaning that 
two simulated seconds will pass for every one real second. This call will have 
no immediate effect if you are not in RealTime mode, but the value will be 
remembered.

@param[in]      simTimePerRealSecond
The number of units of simulation time that should be displayed in one second
of real time. Zero or negative value will be interpeted as the default ratio 
of 1:1. **/
void setRealTimeScale(Real simTimePerRealSecond);
/** Return the current time scale, which will be 1 by default.
@see setRealTimeScale() for more information. **/
Real getRealTimeScale() const;

/** When running an interactive realtime simulation, you can smooth out changes
in simulation run rate by buffering frames before sending them on for 
rendering. The length of the buffer introduces an intentional response time 
lag from when a user reacts to when he can see a response from the simulator. 
Under most circumstances a lag of 150-200ms is undetectable. The default 
buffer length is the time represented by the number of whole frames 
that comes closest to 150ms; 9 frames at 60fps, 5 at 30fps, 4 at 24fps, etc. 
To avoid frequent block/unblocking of the simulation thread, the buffer is
not kept completely full; you can use dumpStats() if you want to see how the
buffer was used during a simulation. Shorten the buffer to improve 
responsiveness at the possible expense of smoothness. Note that the total lag 
time includes not only the buffer length here, but also lag induced by the 
time stepper taking steps that are larger than the frame times. For maximum 
responsiveness you should keep the integrator step sizes limited to about 
100ms, or reduce the buffer length so that worst-case lag doesn't go much over
200ms. 
@param[in]      bufferLengthInSec
This is the target time length for the buffer. The actual length is the nearest
integer number of frames whose frame times add up closest to the request. If
you ask for a non-zero value, you will always get at least one frame in the
buffer. If you ask for zero, you'll get no buffering at all. To restore the
buffer length to its default value, pass in a negative number. **/
void setDesiredBufferLengthInSec(Real bufferLengthInSec);
/** Get the current value of the desired buffer time length the Visualizer 
has been asked to use for smoothing the frame rate, or the default value
if none has been requested. The actual value will differ from this number
because the buffer must contain an integer number of frames. 
@see getActualBufferTime() to see the frame-rounded buffer length **/
Real getDesiredBufferLengthInSec() const;
/** Get the actual length of the real time frame buffer in seconds, which
may differ from the requested time because the buffer contains an integer
number of frames. **/
Real getActualBufferLengthInSec() const;
/** Get the actual length of the real time frame buffer in number of frames. **/
int getActualBufferLengthInFrames() const;

/** Add a new input listener to this Visualizer, methods of which will be
called when the GUI detects user-driven events like key presses, menu picks, 
and slider or mouse moves. See \ref Visualizer::InputListener for more 
information. The Visualizer takes over ownership of the supplied \a listener 
object and deletes it upon destruction of the Visualizer; don't delete it 
yourself. **/
void addInputListener(InputListener* listener);

/** Add a new frame controller to this Visualizer, methods of which will be
called just prior to rendering a frame for the purpose of simulation-controlled
camera positioning and other frame-specific effects. 
See \ref Visualizer::FrameController for more information. The Visualizer takes 
over ownership of the supplied \a controller object and deletes it upon 
destruction of the Visualizer; don't delete it yourself. **/ 
void addFrameController(FrameController* controller);

/**@}**/


/** @name               Frame drawing methods
These are used to report simulation frames to the Visualizer. Typically
the report() method will be called from a Reporter invoked by a TimeStepper, 
but it can also be useful to invoke directly to show preliminary steps in a
simulation, to replay saved States later, and to display frames when using
an Integrator directly rather than through a TimeStepper.

How frames are handled after they have been reported depends on the specific 
method called, and on the Visualizer's current Mode. **/

/**@{**/
/** Report that a new simulation frame is available for rendering. Depending
on the current Visualizer::Mode, handling of the frame will vary:

@par PassThrough
All frames will be rendered, but the calling thread (that is, the simulation) 
may be blocked if the next frame time has not yet been reached or if the 
renderer is unable to keep up with the rate at which frames are being supplied 
by the simulation.

@par Sampling 
The frame will be rendered immediately if the next sample time has been reached
or passed, otherwise the frame will be ignored and report() will return 
immediately.

@par RealTime
Frames are queued to smooth out the time stepper's variable time steps. The 
calling thread may be blocked if the buffer is full, or if the simulation time
is too far ahead of real time. Frames will be dropped if they come too 
frequently; only the ones whose simulated times are at or near a frame time 
will be rendered. Frames that come too late will be queued for rendering as 
soon as possible, and also reset the expected times for subsequent frames so 
that real time operation is restored. **/
void report(const State& state);

/** In RealTime mode there will typically be frames still in the buffer at
the end of a simulation.\ This allows you to wait while the buffer empties. 
When this returns, all frames that had been supplied via report() will have
been sent to the renderer and the buffer will be empty. Returns immediately
if not in RealTime mode, if there is no buffer, or if the buffer is already
empty. **/
void flushFrames();

/** This method draws a frame unconditionally without queuing or checking
the frame rate. Typically you should use the report() method instead, and
let the the internal queuing and timing system decide when to call 
drawFrameNow(). **/
void drawFrameNow(const State& state);
/**@}**/


/** @name                  Scene-building methods
These methods are used to add permanent elements to the scene being displayed
by the Visualizer. Once added, these elements will contribute to every frame.
Calling one of these methods requires writable (non-const) access to the 
Visualizer object; you can't call them from within a FrameController object.
Note that adding DecorationGenerators does allow different
geometry to be produced for each frame; however, once added a 
DecorationGenerator will be called for \e every frame generated. **/
/**@{**/

/** Add a new pull-down menu to the VisualizationGUI's display. The button
label is given in \a title, and a list of (string,int) pairs defines the menu 
and submenu items. The strings have a pathname-like syntax, like "submenu/item1",
"submenu/item2", "submenu/lowermenu/item1", etc. that is used to define the
pulldown menu layout. **/
void addMenu(const String& title, const Array_<std::pair<String, int> >& items);

/** Add a new slider to the VisualizationGUI's display.
@param title    the title to display next to the slider
@param id       an integer value that uniquely identifies this slider
@param min      the minimum value the slider can have
@param max      the maximum value the slider can have
@param value    the initial value of the slider, which must be between min and max **/
void addSlider(const String& title, int id, Real min, Real max, Real value);

/** Add an always-present, body-fixed piece of geometry like the one passed in,
but attached to the indicated body. The supplied transform is applied on top of
whatever transform is already contained in the supplied geometry, and any body 
index stored with the geometry is ignored. **/
void addDecoration(MobilizedBodyIndex, const Transform& X_BD, 
                   const DecorativeGeometry&);

/** Add an always-present rubber band line, modeled after the DecorativeLine 
supplied here. The end points of the supplied line are ignored, however: at 
run time the spatial locations of the two supplied stations are calculated and 
used as end points. **/
void addRubberBandLine(MobilizedBodyIndex b1, const Vec3& station1,
                        MobilizedBodyIndex b2, const Vec3& station2,
                        const DecorativeLine& line);

/** Add a DecorationGenerator that will be invoked to add dynamically generated
geometry to each frame of the the scene. The Visualizer assumes ownership of the 
object passed to this method, and will delete it when the Visualizer is 
deleted. **/
void addDecorationGenerator(DecorationGenerator* generator);
/**@}**/

/** @name                Frame control methods
These methods can be called prior to rendering a frame to control how the 
camera is positioned for that frame. These can be invoked from within a
FrameControl object for runtime camera control and other effects. **/
/**@{**/

/** Set the transform defining the position and orientation of the camera.

@param[in]   X_GC   This is the transform giving the pose of the camera's 
                    frame C in the ground frame G; see below for a precise
                    description.

Our camera uses a right-handed frame with origin at the image location,
with axes oriented as follows: the x axis is to the right, the y axis is the 
"up" direction, and the z axis is the "back" direction; that is, the camera is 
looking in the -z direction. If your simulation coordinate system is different,
such as the common "virtual world" system where ground is the x-y plane 
(x right and y "in") and z is up, be careful to account for that when 
positioning the camera. 

For example, in the virtual world coordinate system, setting \a X_GC to 
identity would put the camera at the ground origin with the x axis as expected,
but the camera would be looking down (your -z) with the camera's "up" direction
aligned with your y. In this case to make the camera look in the y direction 
with up in z, you would need to rotate it +90 degrees about the x axis:
@code
Visualizer viz;
// ...

// Point camera along Ground's y axis with z up, by rotating the camera
// frame's z axis to align with Ground's -y.
viz.setCameraTransform(Rotation(Pi/2, XAxis));
@endcode **/
void setCameraTransform(const Transform& X_GC) const;

/** Move the camera forward or backward so that all geometry in the scene is 
visible. **/
void zoomCameraToShowAllGeometry() const;

/** Rotate the camera so that it looks at a specified point.
@param point        the point to look at
@param upDirection  a direction which should point upward as seen by the camera
**/
void pointCameraAt(const Vec3& point, const Vec3& upDirection) const;

/** Set the camera's vertical field of view, measured in radians. **/
void setCameraFieldOfView(Real fov) const;

/** Set the distance from the camera to the near and far clipping planes. **/
void setCameraClippingPlanes(Real nearPlane, Real farPlane) const;

/** Change the value currently shown on one of the sliders. 
@param slider       the id given to the slider when created
@param value        a new value for the slider; if out of range it will 
                    be at one of the extremes **/
void setSliderValue(int slider, Real value) const;

/** Change the allowed range for one of the sliders. 
@param slider   the id given to the slider when created
@param newMin   the new lower limit on the slider range, <= newMax   
@param newMax   the new upper limit on the slider range, >= newMin
The slider's current value remains unchanged if it still fits in the
new range, otherwise it is moved to the nearest limit. **/
void setSliderRange(int slider, Real newMin, Real newMax) const;

/** Change the title on the main VisualizerGUI window.\ The default title
is the name of the simulation application's executable file. **/
void setWindowTitle(const String& title) const;

/**@}**/

/** @name            Methods for debugging and statistics **/
/**@{**/
/** Dump statistics to the given ostream (e.g. std::cout). **/
void dumpStats(std::ostream& o) const;
/** Reset all statistics to zero. **/
void clearStats();
/**@}**/

/** @name                       Internal use only **/
/**@{**/
const Array_<InputListener*>&   getInputListeners() const;
const Array_<FrameController*>& getFrameControllers() const;
/**@}**/

class VisualizerRep;
//--------------------------------------------------------------------------
                                private:
VisualizerRep* rep;

const VisualizerRep& getRep() const {assert(rep); return *rep;}
VisualizerRep&       updRep()       {assert(rep); return *rep;}
};

/** This abstract class represents an object that will be invoked by the
Visualizer just prior to rendering each frame. You can use this to call any
of the const (runtime) methods of the Visualizer, typically to control the 
camera, and you can also add some geometry to the scene, print messages to 
the console, and so on. **/
class SimTK_SIMBODY_EXPORT Visualizer::FrameController {
public:
    /** The Visualizer is just about to generate and render a frame 
    corresponding to the given State. 
    @param[in]          viz     
        The Visualizer that is doing the rendering.
    @param[in]          state   
        The State that is being used to generate the frame about to be
        rendered by \a viz.
    @param[in,out]      geometry 
        DecorativeGeometry being accumulated for rendering in this frame;
        be sure to \e append if you have anything to add.
    **/
    virtual void generateControls(const Visualizer&           viz, 
                                  const State&                state,
                                  Array_<DecorativeGeometry>& geometry) = 0;

    /** Destructor is virtual; be sure to override it if you have something
    to clean up at the end. **/
    virtual ~FrameController() {}
};

} // namespace SimTK

#endif // SimTK_SIMBODY_VISUALIZER_H_
