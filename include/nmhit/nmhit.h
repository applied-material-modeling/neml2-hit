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

/// Parse HIT input text and return the document root.
///
/// @param fname  path used in error messages (typically the input file path)
/// @param input  complete HIT text
/// @param pre    HIT snippets prepended before @p input (parsed as one document)
/// @param post   HIT snippets appended after  @p input (parsed as one document)
///
/// All content — pre, input, and post — is concatenated and parsed as a single
/// document, so override semantics (':=') apply globally across all sources.
///
/// @throws nmhit::Error on syntax errors
std::unique_ptr<Node> parse(const std::filesystem::path & fname,
                            const std::string & input,
                            const std::vector<std::string> & pre  = {},
                            const std::vector<std::string> & post = {});

} // namespace nmhit
