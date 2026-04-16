#pragma once

/// Umbrella header — include this to use the NEML2-flavored HIT parser.
#include "nmhit/BraceExpr.h"
#include "nmhit/Node.h"
#include "nmhit/TypeRegistry.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace nmhit
{

/// Parse a HIT input file and return the document root.
///
/// @param fname  path to the HIT input file to read and parse
/// @param pre    HIT snippets prepended before the file content (parsed as one document)
/// @param post   HIT snippets appended after  the file content (parsed as one document)
///
/// All content — pre, file, and post — is concatenated and parsed as a single
/// document, so override semantics (':=') apply globally across all sources.
///
/// @throws nmhit::Error if the file cannot be opened or on syntax errors
std::unique_ptr<Node> parse_file(const std::filesystem::path & fname,
                                 const std::vector<std::string> & pre  = {},
                                 const std::vector<std::string> & post = {});

/// Parse HIT text from an in-memory string and return the document root.
///
/// @param input  complete HIT text to parse
/// @param pre    HIT snippets prepended before @p input (parsed as one document)
/// @param post   HIT snippets appended after  @p input (parsed as one document)
///
/// Error messages carry no filename.  @c !include paths are resolved relative
/// to the current working directory.
///
/// @throws nmhit::Error on syntax errors
std::unique_ptr<Node> parse_text(const std::string & input,
                                 const std::vector<std::string> & pre  = {},
                                 const std::vector<std::string> & post = {});

} // namespace nmhit
