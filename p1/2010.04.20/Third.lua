
VertexShaders = { GL2 = {}, GL3 = {}, GL4 = {}, ES2 = {} }
GeometryShaders = { GL3 = {}, GL4 = {} }
FragmentShaders = { GL2 = {}, GL3 = {}, GL4 = {}, ES2 = {} }
TessControlShaders = { GL4 = {} }
TessEvaluationShaders = { GL4 = {} }
ApiVersionToLanguageVersion = { GL2 = 120, GL3 = 130, GL4 = 150 }

function DeclareShader(stage, apiVersion, techniqueName, source)
    local tableName = stage .. "Shaders"
    local languageVersion = ApiVersionToLanguageVersion[apiVersion]
    local lineNumber = debug.getinfo(2).currentline
    _G[tableName][apiVersion][techniqueName] = 
        '#version ' .. languageVersion .. '\n' ..
        '#line ' .. lineNumber .. '\n' .. source
end

DeclareShader('Vertex', 'GL2', 'SnazzyEffect', [[
void main() { /* FOO */ }
]])

DeclareShader('Vertex', 'GL3', 'SnazzyEffect', [[
void main() { /* BAR */ }
]])
