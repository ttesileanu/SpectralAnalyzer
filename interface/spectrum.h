/** @file spectrum.h
 *  @brief This defines the actual spectrum application class.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef SPECTRUM_H_
#define SPECTRUM_H_

#include <vector>

#include <boost/scoped_ptr.hpp>

#include "animation/animator.h"
#include "glutils/fbo.h"
#include "glutils/geometry.h"
#include "glutils/vbo.h"
#include "processor/grabber.h"
#include "sdl/sdl_app.h"
#include "utils/exception.h"
#include "utils/forward_defs.h"
#include "utils/properties.h"

/** @brief The spectrum application class.
 *
 *  This is an OpenGL application that handles various visualizations for sound.
 */
class SpectrumApp : public SdlGlApp {
 public:
  /// This maps names to input modules.
  typedef std::map<std::string, BaseInputPtr> InputChoices;

  /// Constructor.
  SpectrumApp() : properties_(0), display_region_(0, 0, 640, 480),
      display_opacity_(1) {}

  /// Overriding the initialization routine.
  virtual bool init();
  /// Overriding the OpenGL initialization.
  virtual bool initGl();

  /// Overriding the cleanup routine.
  virtual void cleanup();

  /// Overriding event handling.
  virtual void handleEvent(SDL_Event* event);

  /// Overriding the rendering routine.
  virtual void render();

  /// Draw one display on screen.
  void drawDisplay(const BaseSdlDisplayPtr& display, float opac, bool clear);

  /// Select an input module.
  void selectInput(const std::string& name) {
    InputChoices::const_iterator i = input_choices_.find(name);
    if (i == input_choices_.end())
      throw Exception("Unknown input module: " + name +
        " (Spectrum::selectInput).");
    input_.assignBackend(&(*(i -> second)));
    input_name_ = name;
  }

  /// Select a display module.
  void selectDisplay(const std::string& name) {
    SdlDisplays::const_iterator i = displays_.find(name);
    if (i == displays_.end())
      throw Exception("Unknown display module: " + name +
        " (Spectrum::selectDisplay).");
    current_display_ = name;
  }

  /// Add an input module.
  void addInput(const std::string& name, BaseInputPtr input) {
    input_choices_[name] = input;
    if (input_name_.empty()) {
      input_.assignBackend(&(*input));
      input_name_ = name;
    }
  }
  /// Add another processing module, identified by @a name.
  void addProcessor(const std::string& name, BaseProcessorPtr processor)
    { processors_[name] = processor; }
  /// Add another display module.
  void addDisplay(const std::string& name, BaseSdlDisplayPtr display)
    { displays_[name] = display; }

  /// Access the active input module.
  BaseInputPtr getInput() const {
    InputChoices::const_iterator i = input_choices_.find(input_name_);
    return i -> second;
  }

  /// Get the list of input modules.
  const InputChoices& getInputChoices() const { return input_choices_; }

  /// Get the name of the active input module.
  const std::string& getInputName() const { return input_name_; }

  /// Access the display modules.
  const SdlDisplays& getDisplays() const { return displays_; }
  
  /// Access the processors.
  const Processors& getProcessors() const { return processors_; }

  /// Give the app access to its settings.
  void setProperties(Properties* props) { properties_ = props; }

  /// Update the settings. @see setProperties
  void updateProperties();

 private:
  void chooseNextInput();
  void choosePreviousInput();

  InputChoices                  input_choices_;
  std::string                   input_name_;
  Grabber                       input_;
  Processors                    processors_;
  SdlDisplays                   displays_;
  boost::scoped_ptr<Vbo>        vbo_;
  boost::scoped_ptr<Fbo>        fbo_;
  DiscreteAnimated<std::string> current_display_;
  Properties*                   properties_;

  Rectangle                     display_region_;
  float                         display_opacity_;
  Animator                      animator_;
  TransitionStorePtr            transitions_;
};

#endif
