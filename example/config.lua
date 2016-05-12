require('syslog')
require('es')

elasticsearch = es_create("localhost", 9200)

function parse(table, line)
  return table
end

function output(table)
  for k, v in pairs(table) do
    -- print(k..':'..v)
  end
end

config = {
  debug = true,
  steps = {
    {
      filters = {syslog_filter},
      parsers = {syslog_parse},
      outputs = {output}
    }
  }
}
