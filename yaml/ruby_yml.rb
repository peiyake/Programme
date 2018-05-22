#!/usr/local/bin/ruby
require 'YAML'

path="/Users/piak/Documents/github/Programme/yaml/demo.yml"
info=YAML.load(File.open(path))

puts info
puts info.size
puts info.keys
puts info['Device']
puts info['Device'][0]
puts info['Interface']['ensp1']
puts info['Interface'][0]

