#include "interface/spectrum.h"

#include "animation/transition_store.h"
#include "display/base_display.h"
#include "display/oscilloscope.h"
#include "display/spectral_envelope.h"
#include "display/spectrogram.h"
#include "glutils/color.h"
#include "glutils/geometry.h"
#include "input/base_input.h"
#include "input/fake_input.h"
#include "input/pa_input.h"
#include "processor/base_processor.h"
#include "processor/fft.h"
#include "processor/grabber.h"
#include "processor/window_functions.h"
#include "utils/logging.h"
#include "utils/forward_defs.h"

bool SpectrumApp::init()
{
  if (!properties_)
    return false;

  setWidth(properties_ -> get<unsigned>("display.width"));
  setHeight(properties_ -> get<unsigned>("display.height"));

  Properties& input_params = properties_ -> get_child("input");
  Properties& display_params = properties_ -> get_child("display");

  // add the input modules
  const StringVector input_types = splitString(
    input_params.get<std::string>("types"));

  for (StringVector::const_iterator i = input_types.begin();
        i != input_types.end();
        ++i)
  {
    BaseInputPtr input;
    unsigned bufsize = input_params.get<unsigned>((*i) + ".buffer");
    if (*i == "fake") {
      FakeInput* fake_input = new FakeInput(bufsize);

      input = BaseInputPtr(fake_input);
    } else if (*i == "portaudio") {
      PaInput* pa_input = new PaInput(bufsize);

      input = BaseInputPtr(pa_input);
    } else {
      throw Exception("Unknown input module (" + (*i) + ").");
    }

    input -> setProperties(&(input_params.get_child(*i)));
    addInput(*i, input);
  }
  selectInput(input_params.get<std::string>("current"));

  // add the FFT processor
  FftProcessor* fft = new FftProcessor;
  addProcessor("fft", BaseProcessorPtr(fft));

  // add a window function
  // XXX should allow creating of several window functions
  std::string window_type = properties_->get<std::string>("processors.window");
  if (window_type == "gaussian") {
    GaussianWindow* window = new GaussianWindow;
    window -> setProperties(&(properties_->get_child("processors.gaussian")));
    window -> addInput("input", &input_);
    addProcessor("window", BaseProcessorPtr(window));
  } else {
    throw Exception("Unrecognized window function (" + window_type + ").");
  }

  // set the input for the FFT processor
  fft -> addInput("input", &(*processors_["window"]));

  // create the transition store
  transitions_ = boost::make_shared<TransitionStore>();
  transitions_ -> setProperties(&properties_ -> get_child("transitions"));

  // add the display modules
  const StringVector display_types = splitString(
    display_params.get<std::string>("types"));

  for (StringVector::const_iterator i = display_types.begin();
        i != display_types.end();
        ++i)
  {
    BaseSdlDisplayPtr display;
    if (*i == "oscilloscope") {
      Oscilloscope* oscilloscope = new Oscilloscope;

      display = BaseSdlDisplayPtr(oscilloscope);
    } else if (*i == "spectral") {
      SpectralEnvelope* spectral_envelope = new SpectralEnvelope;
      spectral_envelope -> addInput("fft", fft);

       display = BaseSdlDisplayPtr(spectral_envelope);
    } else if (*i == "spectrogram") {
      Spectrogram* spectrogram = new Spectrogram;
      spectrogram -> addInput("fft", fft);

       display = BaseSdlDisplayPtr(spectrogram);
    } else {
      throw Exception("Unknown display module (" + (*i) + ").");
    }

    display -> setProperties(&(display_params.get_child(*i)));
    display -> setTransitionStore(transitions_);
    display -> addInput("raw", &input_);
    addDisplay(*i, display);
  }
  selectDisplay(display_params.get<std::string>("current"));

  // initialize the inputs
  for (InputChoices::iterator i = input_choices_.begin();
        i != input_choices_.end();
        ++i)
  {
    if (!i -> second -> init())
      return false;
  }

  return SdlGlApp::init();
}

bool SpectrumApp::initGl()
{
  // XXX maybe I should separate this into different functions
  logger::detail << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

  // setup the viewport
  glViewport(0, 0, scr_w_, scr_h_);

  // setup the projection matrix
  // make normalized device coordinates identical to eye coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
//  glOrtho(0, scr_w_, 0, scr_h_, -1, 1);
  gluOrtho2D(0, scr_w_, 0, scr_h_);

  // setup the model matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0, 0, 0, 1);

  // need alpha blending for some animations
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // set up the display region.
  animator_.doTransition(&display_region_,
    Rectangle(scr_w_/8, scr_h_/8, 7*scr_w_/8, 7*scr_h_/8),
    Rectangle(0, 0, scr_w_, scr_h_),
    transitions_ -> get("open"));
  animator_.doTransition(&display_opacity_, 0, 1, transitions_ -> get("open"));

  // initialize the VBO
  const size_t vbo_size = 10*sizeof(GlVertex2);
  vbo_.reset(new Vbo(vbo_size));
  vbo_ -> setAutoResize(false); // get notified if this isn't large enough

  // initialize a framebuffer object
  fbo_.reset(new Fbo(scr_w_, scr_h_));

  // initialize the displays
  for (SdlDisplays::const_iterator i = displays_.begin();
        i != displays_.end();
        ++i)
  {
    i -> second -> resize(scr_w_, scr_h_);
    if (i -> second -> init() != 0)
      return false;
  }

  if (!displays_.empty() && current_display_.target.empty())
    current_display_ = displays_.begin() -> first;

  return true;
}

void SpectrumApp::cleanup()
{
  updateProperties();

  // clean up the displays
  for (SdlDisplays::const_iterator i = displays_.begin();
        i != displays_.end();
        ++i)
  {
    i -> second -> done();
  }

  // clean up the inputs
  for (InputChoices::const_iterator i = input_choices_.begin();
        i != input_choices_.end();
        ++i)
  {
    i -> second -> done();
  }

  SdlGlApp::cleanup();
}

void SpectrumApp::handleEvent(SDL_Event* event)
{
  bool handled = false;

  // this can't be end()
  SdlDisplays::const_iterator i = displays_.find(current_display_.target);

  if (event -> type == SDL_KEYUP) {
    switch (event -> key.keysym.sym) {
      case SDLK_ESCAPE:
        running_ = false;
        handled = true;
        break;
      case SDLK_d:
        if (event -> key.keysym.mod == 0) {
          ++i;
          if (i == displays_.end())
            i = displays_.begin();
          // XXX this can be done better...
          current_display_.initial = current_display_.target;
          current_display_.target = i -> first;
          animator_.doTransition(&current_display_.progress, 0, 1,
            transitions_ -> get("disp_fade"));
          handled = true;
        } else if ((event -> key.keysym.mod & KMOD_SHIFT) != 0 &&
          (event -> key.keysym.mod & (~KMOD_SHIFT)) == 0) {
          // if a shift is pressed, but nothing else...
          if (i == displays_.begin())
             i = displays_.end();
          --i;
          current_display_.initial = current_display_.target;
          current_display_.target = i -> first;
          animator_.doTransition(&current_display_.progress, 0, 1,
            transitions_ -> get("disp_fade"));
          handled = true;
        }
        break;
      case SDLK_i:
        if (event -> key.keysym.mod == 0) {
          chooseNextInput();
          handled = true;
        } else if ((event -> key.keysym.mod & KMOD_SHIFT) != 0 &&
          (event -> key.keysym.mod & (~KMOD_SHIFT)) == 0) {
          // if a shift is pressed, but nothing else...
          choosePreviousInput();
          handled = true;
        }
        break;
      default:;
    }
  }

  if (handled)
    return;

  SdlDisplays::const_iterator i0 = i;
  do {
    handled = i -> second -> handleEvent(event);
    ++i;
    if (i == displays_.end())
      i = displays_.begin();
  } while (i != i0 && !handled);

  if (!handled)
    SdlGlApp::handleEvent(event);
}

void SpectrumApp::render()
{
  // update the animations
  animator_.update();

  // let all the processors know that a new display cycle started
  for (Processors::const_iterator j = processors_.begin();
        j != processors_.end();
        ++j)
  {
    j -> second -> invalidateCache();
  }

  SdlDisplays::const_iterator i1 = displays_.find(current_display_.target);
  if (i1 == displays_.end()) {
    logger::error << "Display not found, " << current_display_.target << "."
                  << std::endl;
  }

  const float eps = 1e-6;
  float opacity1 = current_display_.progress;
  if (1 - opacity1 >= eps) {
    // we need to do some blending
    SdlDisplays::const_iterator i2 = displays_.find(current_display_.initial);
    if (i2 == displays_.end()) {
      logger::error << "Display not found, " << current_display_.initial << "."
                    << std::endl;
    }
    drawDisplay(i2 -> second, 1 - opacity1, true);
    drawDisplay(i1 -> second, opacity1, false);
  } else {
    drawDisplay(i1 -> second, opacity1, true);
  }

  swapBuffers();

  // don't eat up unnecessary time
  microDelay(2000);
}

void SpectrumApp::drawDisplay(const BaseSdlDisplayPtr& display, float opac,
  bool clear)
{
  // draw in FBO
  fbo_ -> bind();
  glClear(GL_COLOR_BUFFER_BIT);

  display -> draw();

  // switch back to display
  Fbo::unbind();

  // draw the FBO to screen
  if (clear)
    glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  fbo_ -> getTexture() -> bind();
  opac *= display_opacity_;
  setGlColor(GlColor4(opac, opac, opac, opac));

  // fill the VBO
  std::vector<GlVertexTex2> points_tex;

  float x1 = display_region_.start.x;
  float y1 = display_region_.start.y;
  float x2 = display_region_.end.x;
  float y2 = display_region_.end.y;
  points_tex.push_back(GlVertexTex2(x1, y1, 0, 0));
  points_tex.push_back(GlVertexTex2(x2, y1, 1, 0));
  points_tex.push_back(GlVertexTex2(x2, y2, 1, 1));
  points_tex.push_back(GlVertexTex2(x1, y2, 0, 1));

  // select the texture
  glClientActiveTexture(GL_TEXTURE0);

  // send the data to OpenGL
  vbo_ -> draw(points_tex, GL_QUADS);
  glDisable(GL_TEXTURE_2D);
}

void SpectrumApp::chooseNextInput()
{
  InputChoices::const_iterator i = input_choices_.find(input_name_);
  // here i cannot be == end().
  ++i;

  if (i == input_choices_.end())
    i = input_choices_.begin();

  selectInput(i -> first);
}

void SpectrumApp::choosePreviousInput()
{
  InputChoices::const_iterator i = input_choices_.find(input_name_);
  if (i == input_choices_.begin())
    i = input_choices_.end();
  --i;

  selectInput(i -> first);
}

void SpectrumApp::updateProperties()
{
  properties_ -> put("input.current", input_name_);
  properties_ -> put("display.current", current_display_.target);

  for (InputChoices::iterator i = input_choices_.begin();
        i != input_choices_.end();
        ++i)
  {
    i -> second -> updateProperties();
  }
  for (SdlDisplays::iterator i = displays_.begin(); i != displays_.end(); ++i) {
    i -> second -> updateProperties();
  }
  for (Processors::iterator i = processors_.begin();
        i != processors_.end();
        ++i)
  {
    i -> second -> updateProperties();
  }
}
