/** @file forward_defs.h
 *  @brief This file contains forward definitions.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef FORWARD_DEFS_H_
#define FORWARD_DEFS_H_

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

class BaseDisplay;
/// Smart pointer to a display module.
typedef boost::shared_ptr<BaseDisplay> BaseDisplayPtr;

/// A map from names to display modules.
typedef std::map<std::string, BaseDisplayPtr> Displays;

class BaseSdlDisplay;
/// Smart pointer to an SDL display module.
typedef boost::shared_ptr<BaseSdlDisplay> BaseSdlDisplayPtr;

/// A map from names to SDL display modules.
typedef std::map<std::string, BaseSdlDisplayPtr> SdlDisplays;

class BaseEasing;
/// Smart pointer to an easing function.
typedef boost::shared_ptr<BaseEasing> BaseEasingPtr;

class BaseInput;
/// Smart pointer to an input module.
typedef boost::shared_ptr<BaseInput> BaseInputPtr;

class BaseProcessor;
/// Smart pointer to a signal processor.
typedef boost::shared_ptr<BaseProcessor> BaseProcessorPtr;

/// A map from names to processing modules.
typedef std::map<std::string, BaseProcessorPtr> Processors;

class TransitionStore;
/// Smart pointer to the transition store.
typedef boost::shared_ptr<TransitionStore> TransitionStorePtr;

#endif
