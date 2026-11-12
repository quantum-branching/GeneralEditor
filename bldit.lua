--[[

  Quantix

  Copyright (C) 2026 QuantumBranching
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

]]

bldit_version   = "1.2.0"
package_version = "0.0.0"

dependencies = {
}

targets = {
  default = {
    build = function()
      return os.execute("make")
    end,
    install = function()
      return os.execute("make install")
    end,
    uninstall = function()
      return os.execute("make uninstall")
    end,
  },
  quiet = {
    build = function()
      return os.execute("make &>/tmp/pkgit_build.log")
    end,
    install = function()
      return os.execute("make install".." &>/tmp/pkgit_build.log")
    end,
    uninstall = function()
      return os.execute("make uninstall".." &>/tmp/pkgit_build.log")
    end,
  },
}