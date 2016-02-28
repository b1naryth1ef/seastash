local elasticsearch = require "elasticsearch"

function es_create(host, port)
  local inst = {}


  inst.parser = function(table, line)
    print(inst.client)
    return table
  end

  inst.output = function(table)
  end

  inst.client = elasticsearch.client{hosts={
    {protocol="http", host=host, port=port}
  }}

  return inst
end

