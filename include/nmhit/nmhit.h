#pragma once

/// Umbrella header — include this to use the NEML2-flavored HIT parser.
#include "nmhit/BraceExpr.h"
#include "nmhit/Node.h"
#include "nmhit/TypeRegistry.h"

#include <filesystem>
#include <memory>
#include <string>

namespace nmhit
{

/// Parse HIT input text and return the document root.
/// @param fname  path used in error messages (typically the input file path)
/// @param input  complete HIT text
/// @throws nmhit::Error on syntax errors
std::unique_ptr<Node> parse(const std::filesystem::path & fname, const std::string & input);

} // namespace nmhit
