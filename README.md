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
```lua

config = {
  // The number of worker-coroutines to run. Generally should == numproc
  num_workers = 2,

  // The number of messages to buffer in memory. These will be completely lost if the process dies
  msg_buffer_len = 128,

  // These are the rules for processing
  steps = {
  {
    filters = {my_filter_function},
    parsers = {my_parser_function},
    outputs = {my_output_function}
  }
}
```

## TODO
- Each worker needs some state associated with it, including a lua vm
- Add plugin loading
- Write couple parsers and ES plugin

