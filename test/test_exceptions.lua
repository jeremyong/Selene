function call_protected(function_name)
   return pcall(_ENV[function_name])
end

function raise(exception_message)
   error(exception_message)
end
