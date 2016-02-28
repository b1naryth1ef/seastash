--[[
A library plugin for filtering and parsing RFC5424 syslog messages
]]--

local syslog_format_parts = {
  {priority = "<(%d+)>"},
  {version = "(%d)"},
  {timestamp = " ([^ ]*)"},
  {hostname = " ([^ ]*)"},
  {appname = " ([^ ]*)"},
  {procid = " ([^ ]*)"},
  {msgid = " ([^ ]*)"},
  {structured = " %[([^%]]*)%]"},
  {message = " (.*)"}
}

function keys(tbl)
  local res = {}
  for k, _ in pairs(tbl) do
    table.insert(res, k)
  end
  return res
end

function get_syslog_format()
  local syslog_format = {}
  for i, obj in pairs(syslog_format_parts) do
    for _, entry in pairs(obj) do
      table.insert(syslog_format, entry)
    end
  end
  return table.concat(syslog_format, "")
end

syslog_format = get_syslog_format()

function syslog_filter(line)
  return (string.match(line, syslog_format) ~= nil)
end

function syslog_parse(table, line)
  for k, v in pairs({string.match(line, syslog_format)}) do
    table[keys(syslog_format_parts[k])[1]] = v
  end
  return table
end

