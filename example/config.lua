require('es')

elasticsearch = es_create("localhost", 9200)

function filter(line)
  return true
  -- return string.sub(line, 1, 4) == "asdf"
end

function parse(table, line)
  table.key = string.match(line, "%d+")
  return table
end

function output(table)

end

print(elasticsearch.parser)

config = {
  num_workers = 4,
  msg_buffer_len = 128,

  steps = {
    {
      filters = {filter},
      parsers = {parse, parse},
      outputs = {output}
    }
  }
}
