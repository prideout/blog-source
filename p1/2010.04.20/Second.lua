-- Shader Factory
function DeclareShader(name, source)
    local lineNumber = debug.getinfo(2).currentline
    preamble = "#line " .. lineNumber .. "\n"
    _G[name] = preamble .. source
end

-- Vertex Shader
DeclareShader('MyVertexShader', [[
in vec4 Position;
uniform mat4 Projection;
void main()
{
    gl_Position = Projection * Position;
}]])

-- Fragment Shader
DeclareShader('MyFragmentShader', [[
out vec4 FragColor;
uniform vec4 FillColor;
void main()
{
    FragColor = FillColor;
}]])

--[[

function AddShader3(name, source)
    local lineNumber = debug.getinfo(2).currentline
    local first, last = string.find(debug.getinfo(2).source, "[A-Za-z]+%.glsl")
    local technique = string.sub(debug.getinfo(2).source, first, last - 5) -- removes the '@' prefix and the '.glsl' suffix
    source = "#line " .. lineNumber .. "\n" .. source
    modeIndex = string.find(api, "%.")
    if modeIndex then
        mode = string.sub(api, modeIndex + 1, -1)
        api = string.sub(api, 0, modeIndex - 1)
        technique = technique .. "." .. mode
    end
    if api == 'GL3' then source = "#version 140\n" .. source end
    self[api][technique] = source
end

VertexShader = { GL2 = {}, GL3 = {}, ES2 = {} }
FragmentShader = { GL2 = {}, GL3 = {}, ES2 = {} }

setmetatable(FragmentShader, { __call  =
function(self, api, source)
	local lineNumber = debug.getinfo(2).currentline
	local first, last = string.find(debug.getinfo(2).source, "[A-Za-z]+%.glsl")
	local technique = string.sub(debug.getinfo(2).source, first, last - 5) -- removes the '@' prefix and the '.glsl' suffix
	source = "#line " .. lineNumber .. "\n" .. source
    modeIndex = string.find(api, "%.")
    if modeIndex then
        mode = string.sub(api, modeIndex + 1, -1)
        api = string.sub(api, 0, modeIndex - 1)
        technique = technique .. "." .. mode
    end
    if api == 'GL3' then source = "#version 140\n" .. source end
	self[api][technique] = source
end })

--]]
