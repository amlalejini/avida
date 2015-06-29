#include "apto/core/FileSystem.h"
#include "avida/Avida.h"
#include "avida/core/World.h"

#include "AvidaTools.h"

#include "cAvidaConfig.h"
#include "cUserFeedback.h"
#include "cWorld.h"
#include "cWebViewerDriver.h"
#include "avida/util/CmdLine.h"
#include "kinetic/Kinetic.h"
#include "UI/UI.h"
#include "tools/callbacks.h"

namespace UI = emp::UI;
namespace Kinetic = emp::Kinetic;

extern "C"
void StepDriver(){
   cWebViewerDriver::StepUpdate();
}


struct KineticDriver {
   
  Kinetic::Stage stage;
  Kinetic::Layer layer;
  Kinetic::Animation<KineticDriver> anim;
  UI::ElementSlate doc;

  int last_diff;
  int last_rate;
  KineticDriver() : stage(300, 300, "container"), doc(UI::Slate("emp_base2"))
  {
    stage.Add(layer);
    anim.Setup(this, &KineticDriver::Drive, layer);
    anim.Start();

    doc << "<h1>NEW INTERFACE</h1>"
	<< "Frame rate = " << UI::Live(last_rate) << "<br>"
	<< "Time diff = " << UI::Live(last_diff) << "<br>";
    
    doc.Update();
  }

  void Pause() { anim.Stop(); }
  void Start() { anim.Start(); }

  void Drive(const Kinetic::AnimationFrame & frame) {
    last_diff = frame.time_diff;
    last_rate = frame.frame_rate;
    doc.Update();
    StepDriver();
  }
};

extern "C" 
int main(int argc, char* argv[])
{
  
   Avida::Initialize();
   
   Apto::Map<Apto::String, Apto::String> defs;
   cAvidaConfig* cfg = new cAvidaConfig();
   cUserFeedback feedback;


   Avida::Util::ProcessCmdLineArgs(argc, argv, cfg, defs);

   World* new_world = new World;

   cWorld* world = cWorld::Initialize(cfg, "/", new_world, &feedback, &defs);


   //Any errors produced by trying to initialize the world
   //will be made available to the WebViewerDriver for display
   //We're also going to share the same feedback object. 
   cWebViewerDriver* driver = new cWebViewerDriver(world,feedback);
   if (!driver->Ready()){
      emp::Alert("The driver is not ready to run.  Abort.");
      return 1;
   }

   //Not supporting analyze mode at the moment
   //if (world->GetConfig().ANALYZE_MODE.Get() > 0)
   //   return 0; //Silently fail
   
   emp::Alert("About to run the driver.");
   
   //We're going to transfer control to the emscripten
   //event looper and let the browser set the execution
   //speed to avoid starving the rednering engine
   // emscripten_set_main_loop(StepDriver, 2, 0);

   auto * d = new KineticDriver();

   return 0;
}

