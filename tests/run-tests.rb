#!/usr/bin/env ruby1.8
=begin
/*
 * This file is part of unity-2d
 *
 * Copyright 2011 Canonical Ltd.
 *
 * Authors:
 * - Gerry Boland <gerry.boland@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
=end

# Testability test suite execution script
# 
# Run all tests with
#   $ ruby run-tests.rb

# Check script being run by Ruby 1.8.x, later versions not supported by TDriver
abort("Aborted! Ruby 1.9 not supported, use 1.8") unless RUBY_VERSION < '1.9'

# Add ./misc/lib to the list of library locations - need to calculate absolute path
require 'pathname'
$library_path = File.expand_path(File.dirname(__FILE__)) + '/misc/lib'
$LOAD_PATH.unshift $library_path

# If cmake was called, obtain the path to the built binary directory. If not, we test the
# installed applications instead
binary_dir_file = $library_path + '/../binary_dir.txt'
if File.exists?(binary_dir_file)
    binary_dir = File.open(binary_dir_file).first.strip
    puts 'Running tests on applications contained within ' + binary_dir
    UNITY_2D_SHELL = binary_dir + '/shell/app/unity-2d-shell'
    UNITY_2D_PANEL = binary_dir + '/panel/app/unity-2d-panel'
    UNITY_2D_SPREAD = binary_dir + '/spread/app/unity-2d-spread'
else
    puts 'NOTICE: source not configured, tests will be carried out on *installed* applications!'
    UNITY_2D_SHELL = 'unity-2d-shell'
    UNITY_2D_PANEL = 'unity-2d-panel'
    UNITY_2D_SPREAD = 'unity-2d-spread'
end

# The following line includes the complete tdriver environment 
require 'tdriver'
include TDriverVerify

# Require unit test framework: This enables execution of test cases and also includes assertions (Test::Unit::Assertions)
require 'testhelper'

# List of directories in which to search for test cases
test_directories = ['launcher', 'panel', 'places', 'spread', 'window-manager', 'other']

# Only run scan for tests if this script is directly called
if __FILE__ == $0
    $INIT_COMPLETED = true # Prevent this file being included by test cases

    # Scan through the above directories and execute test cases contained.
    test_directories.each do | directory |
        Dir["#{directory}/*.rb"].each { |testCase| require testCase}
    end
end
