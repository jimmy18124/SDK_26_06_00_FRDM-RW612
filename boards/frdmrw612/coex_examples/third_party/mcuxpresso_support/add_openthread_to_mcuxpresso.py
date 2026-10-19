#/*
# * Copyright 2026 NXP
# *
# * SPDX-License-Identifier: BSD-3-Clause
# *
# * Redistribution and use in source and binary forms, with or without modification,
# * are permitted provided that the following conditions are met:
# *
# * o Redistributions of source code must retain the above copyright notice, this list
# *   of conditions and the following disclaimer.
# *
# * o Redistributions in binary form must reproduce the above copyright notice, this
# *   list of conditions and the following disclaimer in the documentation and/or
# *   other materials provided with the distribution.
# *
# * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
# *   contributors may be used to endorse or promote products derived from this
# *   software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# */

"""
MCUXpresso XML Updater

Update MCUXpresso project XML manifest with OpenThread header files.

Usage:
    python3 add_openthread_to_mcuxpresso.py --app coex_wifi_edgefast
    python3 add_openthread_to_mcuxpresso.py --app coex_wifi_edgefast --dry-run
    python3 add_openthread_to_mcuxpresso.py --app coex_wifi_edgefast --verbose

Requires Python 3.6+
"""

import argparse
import json
import os
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from datetime import datetime
from collections import defaultdict
import re
import shutil
import platform

# Python version check - MUST be Python 3.6+
if sys.version_info < (3, 6):
    print("="*80)
    print("ERROR: Python 3.6 or higher is required!")
    print("="*80)
    print("Current Python version: " + sys.version)
    print("")
    print("This script uses features that require Python 3.6+:")
    print("  - f-strings (Python 3.6+)")
    print("  - pathlib (enhanced in Python 3.4+)")
    print("  - Modern type hints and syntax")
    print("")
    print("Please use one of the following commands:")
    print("  - python3 add_openthread_to_mcuxpresso.py --app coex_wifi_edgefast")
    print("  - ./add_openthread_to_mcuxpresso.py --app coex_wifi_edgefast")
    print("="*80)
    sys.exit(1)

# Constants
MAX_DIR_SEARCH_DEPTH = 10  # Maximum depth when searching for middleware directory
MAX_DEPENDENCY_DEPTH = 10  # Maximum depth for recursive header dependency analysis

# Platform-specific configuration files to exclude
PLATFORM_CONFIGS = [
    "mcxw70-mbedtls-config.h",
    "mcxw71-mbedtls-config.h",
    "mcxw72-mbedtls-psa-config.h",
    "rt1060-mbedtls-psa-config.h",
    "rt1170-mbedtls-psa-config.h"
]

# Files to exclude from mbedtls3x/port directory
PORT_EXCLUDED = [
    "mcux_mbedtls_config.h",
    "mcux_mbedtls_psa_crypto_config.h",
    "mcux_psa_defines.h"
]

def calculate_relative_path(ot_nxp_root, xml_dir):
    """Calculate relative path from XML directory to ot-nxp root.
    
    Uses pathlib for cross-platform compatibility.
    
    Args:
        ot_nxp_root: Path to ot-nxp directory (Path object or string)
        xml_dir: Path to XML file directory (Path object or string)
    
    Returns:
        Relative path string with forward slashes (MCUXpresso XML format)
    """
    try:
        ot_nxp_path = Path(ot_nxp_root)
        xml_path = Path(xml_dir)
        rel_path = os.path.relpath(ot_nxp_path, xml_path)
        # Convert to forward slashes for XML (MCUXpresso requirement)
        return rel_path.replace(os.sep, '/')
    except (ValueError, OSError):
        # Fallback for cross-drive paths on Windows
        return "third_party/mcuxpresso_support/ot-nxp"


class HeaderDependencyAnalyzer:
    """Analyze header file dependencies with caching and cross-platform support."""
    
    def __init__(self, ot_nxp_root, max_depth=MAX_DEPENDENCY_DEPTH, verbose=False):
        self.ot_nxp_root = Path(ot_nxp_root)
        self.dependencies = defaultdict(set)
        self.processed = set()
        self.system_headers = set()
        self.max_depth = max_depth
        self.verbose = verbose
        self._file_cache = {}  # Cache for parsed files
    
    def find_header_file(self, header_name, search_paths):
        """Find a header file in the search paths.
        
        Uses pathlib for cross-platform path handling.
        """
        for search_path in search_paths:
            header_path = self.ot_nxp_root / search_path / header_name
            if header_path.exists():
                return header_path
        
        # Try as relative path
        if "/" in header_name or "\\" in header_name:
            for search_path in search_paths:
                header_path = self.ot_nxp_root / search_path / header_name
                if header_path.exists():
                    return header_path
        
        return None
    
    def parse_includes(self, file_path):
        """Parse #include statements from a header file with caching."""
        file_key = str(file_path)
        if file_key in self._file_cache:
            return self._file_cache[file_key]
        
        includes = []
        try:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                for line in f:
                    line = line.strip()
                    match = re.match(r'^#include\s+["<]([^">]+)[">]', line)
                    if match:
                        includes.append(match.group(1))
        except (IOError, UnicodeDecodeError, OSError):
            pass
        
        self._file_cache[file_key] = includes
        return includes
    
    def is_system_header(self, header_name):
        """Check if a header is a system header."""
        system_prefixes = ["stdio", "stdlib", "string", "stdint", "stddef", 
                          "stdbool", "limits", "fsl_", "stdarg", "time"]
        return any(header_name.startswith(prefix) for prefix in system_prefixes)
    
    def analyze_dependencies(self, header_file, search_paths, depth=0):
        """Recursively analyze dependencies of a header file."""
        if depth > self.max_depth:
            return
        
        try:
            rel_path = header_file.relative_to(self.ot_nxp_root)
            # Use forward slashes for consistency
            rel_path_str = str(rel_path).replace(os.sep, '/')
        except ValueError:
            return
        
        if rel_path_str in self.processed:
            return
        
        self.processed.add(rel_path_str)
        
        if self.verbose and depth == 0:
            print(f"  Analyzing: {rel_path_str}")
        
        includes = self.parse_includes(header_file)
        
        for include in includes:
            if self.is_system_header(include):
                self.system_headers.add(include)
                continue
            
            included_file = self.find_header_file(include, search_paths)
            if included_file:
                try:
                    inc_rel_path = included_file.relative_to(self.ot_nxp_root)
                    inc_rel_path_str = str(inc_rel_path).replace(os.sep, '/')
                    self.dependencies[rel_path_str].add(inc_rel_path_str)
                    self.analyze_dependencies(included_file, search_paths, depth + 1)
                except ValueError:
                    pass
    
    def get_headers_by_directory(self):
        """Group all discovered headers by directory with filtering.
        
        Returns paths with forward slashes for XML consistency.
        """
        headers_by_dir = defaultdict(set)
        
        excluded_dirs = [
            "mbedtls3x/include/mbedtls",
            "mbedtls3x/include/psa"
        ]
        
        port_whitelist = ["mcux_mbedtls_accelerator_config.h"]
        
        for header in self.processed:
            # header already uses forward slashes from analyze_dependencies
            dir_name = str(Path(header).parent).replace(os.sep, '/')
            file_name = Path(header).name
            
            if any(excluded in dir_name for excluded in excluded_dirs):
                continue
            
            if file_name in PLATFORM_CONFIGS:
                continue
            
            if "mbedtls3x/port" in dir_name:
                if file_name not in port_whitelist:
                    continue
            
            headers_by_dir[dir_name].add(file_name)
        
        # Special handling for openthread headers
        if "openthread/include/openthread" in headers_by_dir:
            headers_by_dir["openthread/include"] = set()
        
        return headers_by_dir


class XMLUpdater:
    """Updates MCUXpresso XML manifest with cross-platform path support."""
    
    def __init__(self, xml_path, verbose=False, dry_run=False):
        self.xml_path = Path(xml_path)
        self.verbose = verbose
        self.dry_run = dry_run
        
        ET.register_namespace("ksdk", "http://nxp.com/ksdk/2.0/ksdk_manifest_v3.0.xsd")
        ET.register_namespace("xsi", "http://www.w3.org/2001/XMLSchema-instance")
        
        self.tree = ET.parse(xml_path)
        self.root = self.tree.getroot()
    
    def create_backup(self, suffix="backup"):
        """Create a timestamped backup."""
        if self.dry_run:
            if self.verbose:
                print(f"[DRY-RUN] Would create backup: {suffix}")
            return None
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_path = self.xml_path.parent / f"{self.xml_path.stem}.{suffix}_{timestamp}.xml"
        shutil.copy2(self.xml_path, backup_path)
        if self.verbose:
            print(f"Created backup: {backup_path.name}")
        return backup_path
    
    def find_include_paths_section(self):
        """Find the include_paths section in XML."""
        for elem in self.root.iter():
            if elem.tag.endswith("include_paths"):
                return elem
        return None
    
    def find_insertion_point_for_source(self):
        """Find where to insert source groups."""
        doc_elements = []
        for elem in self.root.iter():
            if elem.tag.endswith("source") and elem.get("type") == "doc":
                doc_elements.append(elem)
        
        if doc_elements:
            parent = None
            for p in self.root.iter():
                if doc_elements[-1] in list(p):
                    parent = p
                    break
            return parent, doc_elements[-1]
        
        for elem in self.root.iter():
            if elem.tag.endswith("example"):
                return elem, None
        
        return None, None
    
    def remove_ot_nxp_content(self):
        """Remove all existing ot-nxp content."""
        removed_paths = 0
        removed_groups = 0
        
        if self.dry_run:
            if self.verbose:
                print("[DRY-RUN] Would remove existing ot-nxp content")
            return 0, 0
        
        include_paths_section = self.find_include_paths_section()
        if include_paths_section is not None:
            to_remove = []
            for child in list(include_paths_section):
                if isinstance(child.tag, str) and child.tag.endswith("include_path"):
                    path = child.get("path", "")
                    if "ot-nxp" in path:
                        to_remove.append(child)
            for elem in to_remove:
                include_paths_section.remove(elem)
            removed_paths = len(to_remove)
            if self.verbose:
                print(f"  Removed {removed_paths} old include paths")
        
        parent, _ = self.find_insertion_point_for_source()
        if parent is not None:
            to_remove = []
            for child in list(parent):
                if isinstance(child.tag, str) and child.tag.endswith("source"):
                    path = child.get("path", "")
                    if "ot-nxp" in path and child.get("type") == "c_include":
                        to_remove.append(child)
                elif not isinstance(child.tag, str):
                    try:
                        if hasattr(child, "text") and child.text and "OpenThread" in child.text:
                            to_remove.append(child)
                    except:
                        pass
            for elem in to_remove:
                parent.remove(elem)
            removed_groups = len(to_remove)
            if self.verbose:
                print(f"  Removed {removed_groups} old source groups")
        
        return removed_paths, removed_groups
    
    def add_include_paths(self, directories, ot_nxp_root):
        """Add include paths with cross-platform path handling."""
        if self.dry_run:
            if self.verbose:
                print(f"[DRY-RUN] Would add {len(directories)} include paths")
            return len(directories)
        
        include_paths_section = self.find_include_paths_section()
        if include_paths_section is None:
            return 0
        
        # Calculate relative path from XML location to ot-nxp
        xml_dir = self.xml_path.parent
        rel_to_ot_nxp = calculate_relative_path(ot_nxp_root, xml_dir)
        
        added = 0
        for dir_path in sorted(directories):
            # Combine paths using forward slashes (MCUXpresso XML requirement)
            path = f"{rel_to_ot_nxp}/{dir_path}"
            proj_rel_path = f"third_party/mcuxpresso_support/ot-nxp/{dir_path}"
            
            include_path = ET.Element("include_path")
            include_path.set("path", path)
            include_path.set("project_relative_path", proj_rel_path)
            include_path.set("type", "c_include")
            
            include_paths_section.append(include_path)
            added += 1
        
        return added
    
    def add_source_groups(self, headers_by_dir, ot_nxp_root):
        """Add source groups with cross-platform path handling."""
        if self.dry_run:
            total_headers = sum(len(h) for h in headers_by_dir.values())
            if self.verbose:
                print(f"[DRY-RUN] Would add {len(headers_by_dir)} source groups with {total_headers} headers")
            return len(headers_by_dir), total_headers
        
        parent, insert_after = self.find_insertion_point_for_source()
        if parent is None:
            return 0, 0

        # Calculate relative path from XML location to ot-nxp
        xml_dir = self.xml_path.parent
        rel_to_ot_nxp = calculate_relative_path(ot_nxp_root, xml_dir)
        
        insert_index = None
        if insert_after is not None:
            children = list(parent)
            insert_index = children.index(insert_after) + 1
        
        added_groups = 0
        added_headers = 0
        
        excluded_files = set(PLATFORM_CONFIGS) | {"time.h"}
        
        port_whitelist = {"mcux_mbedtls_accelerator_config.h"}
        
        for dir_path in sorted(headers_by_dir.keys()):
            # Use pathlib for file system operations
            full_dir = Path(ot_nxp_root) / dir_path.replace('/', os.sep)
            all_headers = set()
            if full_dir.exists():
                for file in full_dir.iterdir():
                    if file.is_file() and file.suffix == ".h":
                        if file.name in excluded_files:
                            continue
                        if "mbedtls3x/port" in dir_path:
                            if file.name not in port_whitelist:
                                continue
                        all_headers.add(file.name)
            
            for header in headers_by_dir[dir_path]:
                if header in excluded_files:
                    continue
                if "mbedtls3x/port" in dir_path:
                    if header not in port_whitelist:
                        continue
                all_headers.add(header)
            
            if not all_headers:
                continue
            
            # Use forward slashes for XML paths
            path = f"{rel_to_ot_nxp}/{dir_path}"
            proj_rel_path = f"third_party/mcuxpresso_support/ot-nxp/{dir_path}"
            
            comment_text = f" OpenThread {dir_path.split('/')[-1]} Headers "
            comment = ET.Comment(comment_text)
            
            source = ET.Element("source")
            source.set("path", path)
            source.set("project_relative_path", proj_rel_path)
            source.set("type", "c_include")
            
            for filename in sorted(all_headers):
                files_elem = ET.Element("files")
                files_elem.set("mask", filename)
                source.append(files_elem)
            
            if insert_index is not None:
                parent.insert(insert_index, comment)
                parent.insert(insert_index + 1, source)
                insert_index += 2
            else:
                parent.append(comment)
                parent.append(source)
            
            added_groups += 1
            added_headers += len(all_headers)
        
        return added_groups, added_headers
    
    def save(self):
        """Save the XML file."""
        if self.dry_run:
            if self.verbose:
                print("[DRY-RUN] Would save XML file")
            return
        
        self._prettify_xml(self.root)
        self.tree.write(self.xml_path, encoding="UTF-8", xml_declaration=True)
    
    def _prettify_xml(self, elem, level=0):
        """Add proper indentation to XML."""
        indent = "\n" + "  " * level
        if len(elem):
            if not elem.text or not elem.text.strip():
                elem.text = indent + "  "
            if not elem.tail or not elem.tail.strip():
                elem.tail = indent
            for child in elem:
                self._prettify_xml(child, level + 1)
            if not child.tail or not child.tail.strip():
                child.tail = indent
        else:
            if level and (not elem.tail or not elem.tail.strip()):
                elem.tail = indent


def find_xml_file_auto(app_name, script_location=None):
    """
    Automatically find XML file based on application name.

    Uses pathlib for cross-platform path handling.

    Expected layout:
      boards/<board>/coex_examples/third_party/mcuxpresso_support/
      boards/<board>/coex_examples/<app>/*.xml
      boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp/

    ot-nxp is always next to the script.
    XML is in the app directory under coex_examples/.
    """
    if script_location is None:
        script_location = Path(__file__).resolve()
    else:
        script_location = Path(script_location).resolve()

    # ot-nxp is always next to the script
    ot_nxp_root = script_location.parent / "ot-nxp"
    if not ot_nxp_root.exists():
        raise FileNotFoundError(f"ot-nxp directory not found at: {ot_nxp_root}")

    # boards/<board>/coex_examples/third_party/mcuxpresso_support/
    script_dir = script_location.parent    # mcuxpresso_support/
    third_party_dir = script_dir.parent    # third_party/
    coex_examples_dir = third_party_dir.parent  # coex_examples/
    
    # Find the app directory under coex_examples/
    app_dir = coex_examples_dir / app_name
    if not app_dir.exists():
        raise FileNotFoundError(
            f"Cannot locate app directory: {app_dir}. "
            "Script must be in: boards/<board>/coex_examples/third_party/mcuxpresso_support/"
        )

    # Find XML file
    xml_files = [f for f in app_dir.glob("*.xml") if "backup" not in f.name.lower()]

    if not xml_files:
        raise FileNotFoundError(f"No XML file found in: {app_dir}")

    if len(xml_files) > 1:
        matching = [f for f in xml_files if app_name in f.name]
        xml_file = matching[0] if matching else xml_files[0]
    else:
        xml_file = xml_files[0]

    return xml_file, ot_nxp_root, app_dir


def generate_change_report(xml_file, headers_by_dir, stats, ot_nxp_root):
    """Generate detailed change report."""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    report_file = xml_file.parent / f"XML_CHANGE_REPORT_{timestamp}.md"
    
    platform_configs = [
        "mcxw70-mbedtls-config.h", "mcxw71-mbedtls-config.h",
        "mcxw72-mbedtls-psa-config.h", "rt1060-mbedtls-psa-config.h",
        "rt1170-mbedtls-psa-config.h"
    ]
    
    port_excluded = [
        "mcux_mbedtls_config.h",
        "mcux_mbedtls_psa_crypto_config.h",
        "mcux_psa_defines.h"
    ]
    
    with open(report_file, "w", encoding="utf-8") as f:
        f.write("# MCUXpresso XML Change Report\n\n")
        f.write(f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
        f.write(f"**XML File:** `{xml_file.name}`\n\n")
        f.write("---\n\n")
        
        # Summary
        f.write("## Summary\n\n")
        f.write(f"- **Include Paths Added:** {stats['added_paths']}\n")
        f.write(f"- **Source Groups Added:** {stats['added_groups']}\n")
        f.write(f"- **Header Files Added:** {stats['added_headers']}\n")
        f.write(f"- **Directories with Headers:** {len(headers_by_dir)}\n")
        f.write("\n---\n\n")
        
        # Calculate path format
        xml_dir = xml_file.parent
        rel_to_ot_nxp = calculate_relative_path(ot_nxp_root, xml_dir)
        
        # Include paths
        f.write("## Added Include Paths\n\n")
        f.write(f"Total: **{stats['added_paths']}** paths\n\n")
        for i, dir_path in enumerate(sorted(headers_by_dir.keys()), 1):
            path = f"{rel_to_ot_nxp}/{dir_path}"
            f.write(f"{i}. `{path}`\n")
        f.write("\n---\n\n")
        
        # Source groups with details
        f.write("## Added Source Groups and Header Files\n\n")
        f.write(f"Total: **{stats['added_groups']}** groups, **{stats['added_headers']}** header files\n\n")
        
        for dir_path in sorted(headers_by_dir.keys()):
            # Use pathlib for file operations
            full_dir = Path(ot_nxp_root) / dir_path.replace('/', os.sep)
            all_headers = set(headers_by_dir[dir_path])
            
            # Scan directory for all headers
            if full_dir.exists():
                excluded = set(platform_configs + port_excluded + ["time.h"])
                port_whitelist = {"mcux_mbedtls_accelerator_config.h"}
                
                for file in full_dir.iterdir():
                    if file.is_file() and file.suffix == ".h":
                        if file.name in excluded:
                            continue
                        if "mbedtls3x/port" in dir_path and file.name not in port_whitelist:
                            continue
                        all_headers.add(file.name)
            
            if all_headers:
                dir_name = dir_path.split('/')[-1]
                f.write(f"### {dir_name}\n\n")
                f.write(f"**Path:** `{rel_to_ot_nxp}/{dir_path}`\n\n")
                f.write(f"**Header Files:** {len(all_headers)}\n\n")
                f.write("<details>\n")
                f.write(f"<summary>Click to expand header list ({len(all_headers)} files)</summary>\n\n")
                for header in sorted(all_headers):
                    f.write(f"- `{header}`\n")
                f.write("\n</details>\n\n")
        
        f.write("---\n\n")
        
        # Filtered content
        f.write("## Filtered Content (Not Added)\n\n")
        f.write("### Platform-Specific Configuration Files\n\n")
        f.write("These files are for other platforms and were excluded:\n\n")
        for cfg in platform_configs:
            f.write(f"- [EXCLUDED] `{cfg}`\n")
        
        f.write("\n### mbedtls3x/port Non-Essential Files\n\n")
        f.write("These files were excluded from `mbedtls3x/port` directory:\n\n")
        for cfg in port_excluded:
            f.write(f"- [EXCLUDED] `{cfg}`\n")
        f.write("\n**Kept in mbedtls3x/port:**\n")
        f.write("- [INCLUDED] `mcux_mbedtls_accelerator_config.h`\n")
        
        f.write("\n### Excluded Directories\n\n")
        f.write("All header files in these directories were excluded:\n\n")
        f.write("- [EXCLUDED] `mbedtls3x/include/mbedtls` (~74 files)\n")
        f.write("- [EXCLUDED] `mbedtls3x/include/psa` (~23 files)\n")
        
        f.write("\n---\n\n")
        
        # Path format info
        f.write("## Path Format Information\n\n")
        f.write(f"**File System Path (path attribute):** `{rel_to_ot_nxp}`\n\n")
        f.write("**Project Relative Path (project_relative_path attribute):** `third_party/mcuxpresso_support/ot-nxp`\n\n")
        f.write("**Note:** MCUXpresso IDE requires two different path formats:\n")
        f.write("- `path`: Actual file system path from XML location to resource\n")
        f.write("- `project_relative_path`: Logical path within the project structure\n")
        
        f.write("\n---\n\n")
        
        # Dependency analysis
        f.write("## Dependency Analysis Details\n\n")
        f.write("**Entry Configuration Files:**\n\n")
        f.write("1. `ot-nxp-mbedtls-config.h`\n")
        f.write("2. `rw612-mbedtls-psa-config.h`\n")
        f.write("\n**Analysis Method:**\n")
        f.write("- Recursively parsed `#include` statements\n")
        f.write("- Maximum recursion depth: 10 levels\n")
        f.write("- Automatically skipped system headers\n")
        f.write("- Applied smart filtering rules\n")
        
        f.write("\n---\n\n")
        f.write("**Report Generated by:** MCUXpresso XML Updater\n")
    
    return report_file


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="MCUXpresso XML Updater - Update XML manifest with OpenThread headers",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic usage
  python3 %(prog)s --app coex_wifi_edgefast
  
  # Dry-run mode (preview changes)
  python3 %(prog)s --app coex_wifi_edgefast --dry-run
  
  # Verbose output
  python3 %(prog)s --app coex_wifi_edgefast --verbose
"""
    )
    
    parser.add_argument(
        "--app",
        dest="app_name",
        help="Application name (e.g., coex_wifi_edgefast)"
    )
    
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview changes without modifying files"
    )
    
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Verbose output"
    )
    
    return parser.parse_args()


def main():
    """Main function."""
    args = parse_arguments()
    
    print("=" * 80)
    print("MCUXpresso XML Updater")
    print(f"Python: {sys.version.split()[0]}")
    if args.dry_run:
        print("[DRY-RUN MODE - No files will be modified]")
    print("=" * 80)
    
    # Step 1: Find XML file and ot-nxp root
    print("\nStep 1: Locating files...")
    try:
        if not args.app_name:
            print("ERROR: --app parameter is required")
            print("Usage: python3 add_openthread_to_mcuxpresso.py --app <app_name>")
            print("Example: python3 add_openthread_to_mcuxpresso.py --app coex_wifi_edgefast")
            return 1
        
        xml_file, ot_nxp_root, app_dir = find_xml_file_auto(args.app_name)
        print(f"  Found XML: {xml_file.relative_to(app_dir.parent.parent)}")
        print(f"  App directory: {app_dir}")
    
    except FileNotFoundError as e:
        print(f"\nERROR: {e}")
        return 1
    
    print(f"  ot-nxp: {ot_nxp_root}")
    
    # Step 2: Parse compile_commands.json
    print("\nStep 2: Parsing compile_commands.json...")
    compile_commands = ot_nxp_root / "build_rw612" / "rw612_ot_cli" / "compile_commands.json"
    
    if not compile_commands.exists():
        print(f"ERROR: {compile_commands} not found")
        return 1
    
    with open(compile_commands, encoding="utf-8") as f:
        commands = json.load(f)
    
    search_paths = []
    for entry in commands:
        command = entry.get("command", "")
        parts = command.split()
        for i, part in enumerate(parts):
            if part.startswith("-I"):
                if part == "-I" and i + 1 < len(parts):
                    include_path = parts[i + 1]
                else:
                    include_path = part[2:]
                
                try:
                    abs_path = Path(include_path)
                    if abs_path.is_absolute():
                        path_str = str(abs_path)
                        if "ot-nxp" in path_str:
                            # Split on ot-nxp and get relative part
                            parts_list = path_str.split("ot-nxp")
                            if len(parts_list) > 1:
                                # Convert to forward slashes for consistency
                                rel_path = parts_list[1].lstrip("/\\").replace("\\", "/")
                                if rel_path and rel_path not in search_paths:
                                    search_paths.append(rel_path)
                    else:
                        rel_path = str(Path(include_path).relative_to(ot_nxp_root)).replace(os.sep, "/")
                        if rel_path not in search_paths:
                            search_paths.append(rel_path)
                except (ValueError, OSError):
                    pass
    
    print(f"  Found {len(search_paths)} include paths")
    
    # Step 3: Analyze dependencies
    print("\nStep 3: Analyzing header dependencies...")
    analyzer = HeaderDependencyAnalyzer(ot_nxp_root, 10, args.verbose)
    
    config_files = [
        ot_nxp_root / "third_party" / "mbedtls" / "configs" / "ot-nxp-mbedtls-config.h",
        ot_nxp_root / "third_party" / "mbedtls" / "configs" / "rw612-mbedtls-psa-config.h"
    ]
    
    for config_file in config_files:
        if config_file.exists():
            analyzer.analyze_dependencies(config_file, search_paths)
    
    headers_by_dir = analyzer.get_headers_by_directory()
    print(f"  Found {len(headers_by_dir)} directories with headers")
    print(f"  Total headers: {sum(len(h) for h in headers_by_dir.values())}")
    
    # Step 4: Update XML
    print("\nStep 4: Updating XML manifest...")
    
    if not args.dry_run:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        initial_backup = xml_file.parent / f"{xml_file.stem}.original_backup_{timestamp}.xml"
        shutil.copy2(xml_file, initial_backup)
        print(f"  Created backup: {initial_backup.name}")
    
    updater = XMLUpdater(xml_file, args.verbose, args.dry_run)
    updater.remove_ot_nxp_content()
    
    added_paths = updater.add_include_paths(headers_by_dir.keys(), ot_nxp_root)
    added_groups, added_headers = updater.add_source_groups(headers_by_dir, ot_nxp_root)
    
    print(f"  Include paths: {added_paths}")
    print(f"  Source groups: {added_groups}")
    print(f"  Header files: {added_headers}")
    
    updater.save()
    
    # Step 5: Generate report
    if not args.dry_run:
        print("\nStep 5: Generating change report...")
        stats = {
            "added_paths": added_paths,
            "added_groups": added_groups,
            "added_headers": added_headers
        }
        report_file = generate_change_report(xml_file, headers_by_dir, stats, ot_nxp_root)
        print(f"  Report: {report_file.name}")
    
    print("\n" + "=" * 80)
    if args.dry_run:
        print("DRY-RUN completed. No files were modified.")
    else:
        print("Done! XML file updated successfully.")
    print("=" * 80)
    
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nInterrupted by user.")
        sys.exit(1)
    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
