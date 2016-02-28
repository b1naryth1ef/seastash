# SeaStash

Seastash is a C-based logstash-like server for the aggregation, filtering, parsing, and exportation of raw log lines. It uses [libmill](http://libmill.org/) for concurrency, and [lua](http://www.lua.org/) for dynamically configurable log parsing and processing.

## Configuration
```lua

-- Takes a log_line and returns true if this matches (e.g. continue on the pipeline)
def my_filter_function(log_line)
  return false
end


--[[
Takes a table and a log line, returning the table (optionally modified within the function).
The table is passed down through all the parser functions allowing more dynamic forms of chaining
functionality. Finally the last parser will have its returned table passed to the outputs
]]--
def my_parser_function(table, log_line)
  return table
end

-- Takes the completed table and returns nothing
def my_output_function(table)

end

config = {
  -- The number of worker-coroutines to run. Generally should == numproc
  num_workers = 2,

  -- The number of messages to buffer in memory. These will be completely lost if the process dies
  msg_buffer_len = 128,

  -- These are the rules for processing
  steps = {
  {
    filters = {my_filter_function},
    parsers = {my_parser_function},
    outputs = {my_output_function}
  }
}
```

## TODO
- Add support for UDP
- Add a ES output

