function filter(line)
  return string.sub(line, 1, 4) == "asdf"
end

function parse(table, line)
  table.key = string.match(line, "%d+")
  return table
end

function output(tbl)
end

config = {
  num_workers = 4,
  msg_buffer_len = 128,

  steps = {
    {
      filters = {filter},
      parsers = {parse},
      outputs = {output}
    }
  }
}
