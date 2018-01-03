/** @file properties.h
 *  @brief Defines some typedefs for working with property trees.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>

/// A property tree.
typedef boost::property_tree::ptree Properties;

using boost::property_tree::read_xml;

/** @brief My own version of @a write_xml, because the default one has
 *  weird formatting.
 */
void my_write_xml(const std::string& fname, const Properties& props);

#endif
