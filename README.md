# SeaStash

SeaStash is a C implementation of the logstash server that was built for extremely fast and efficient log processing. It uses [libmill](http://libmill.org/) for concurrency, and [lua](http://www.lua.org/) for dynamically configurable log parsing and processing.

## Writing Parsing Plugins
```lua

// If false we'll skip the rest of this plugin
function filter_log(line)
  return true
end

// Should return a json-encodable table
function parse_log(line)
  return {}
end
```

## Writing Output Plugins
```lua


// If your output does not support batching use this
function output_log(log)

end

// Otherwise use this, logs is a table with N many parsed logs
function output_log_batch(logs)

end
```

## Configuration
```
config = {
  steps: {
    {filters: {plugin1, plugin2}, outputs: {plugin3}},
    {filters: {plugin4}, outputs: {plugin3, plugin5}}
  }
}
```

## TODO
- Each worker needs some state associated with it, including a lua vm
- Add plugin loading
- Write couple parsers and ES plugin

