#!/usr/bin/env ruby
#
# Testability test suite execution script
# 
# Run all tests with
#   $ ruby run-tests.rb

require './misc/lib/testhelper.rb'

# List of directories in which to search for test cases
test_directories = ['launcher', 'panel', 'places', 'spread', 'window-manager', 'other']

# Scan through the above directories and execute test cases contained.
test_directories.each do | directory |
    Dir["#{directory}/*.rb"].each { |testCase| require testCase}
end

