______  _  _  _   
| ___ \| |(_)| |  
| |_/ /| | _ | |_ 
| ___ \| || || __|
| |_/ /| || || |_ 
\____/ |_||_| \__|
                  
-- Vertex.GL2.Blit --

// Vertex.GL2.Blit \\

attribute vec4 Position;
void main()
{
    gl_Position = Position;
}

-- Fragment.GL2.Blit --

// Fragment.GL2.Blit \\

uniform usampler2D Sampler;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    gl_FragColor = texelFetch(Sampler, coord, 0).xy;
}

--
 _____                  _               
|  ___|                (_)              
| |__  _ __  ___   ___  _   ___   _ __  
|  __|| '__|/ _ \ / __|| | / _ \ | '_ \ 
| |___| |  | (_) |\__ \| || (_) || | | |
\____/|_|   \___/ |___/|_| \___/ |_| |_|
                                        
---Vertex.GL3.Erosion <== This is a section divider

// -- this does not count as a section divider

// Vertex.GL3.Erosion \\

in vec4 Position;
void main()
{
    gl_Position = Position;
}

----[[[ Fragment.GL3.Erosion.HorizontalRed]]]----

// Fragment.GL3.Erosion.HorizontalRed \\

out uvec2 FragColor;
uniform usampler2D Sampler;
uniform uint Beta;
uniform uint Infinity;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);

    uvec2 A = texelFetch(Sampler, coord, 0).rg;
    float E = texelFetchOffset(Sampler, coord, 0, ivec2(+1, 0)).r;
    float W = texelFetchOffset(Sampler, coord, 0, ivec2(-1, 0)).r;
    uvec2 B = uvec2(min(min(A.r, E + Beta), W + Beta), A.g);

    if (A == B || B.r > Infinity)
        discard;

    FragColor = B;
}

-- Fragment.GL3.Erosion.HorizontalGreen --

// Fragment.GL3.Erosion.HorizontalGreen \\

out uvec2 FragColor;
uniform usampler2D Sampler;
uniform uint Beta;
uniform uint Infinity;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);

    uvec2 A = texelFetch(Sampler, coord, 0).rg;
    uint E = texelFetchOffset(Sampler, coord, 0, ivec2(+1, 0)).g;
    uint W = texelFetchOffset(Sampler, coord, 0, ivec2(-1, 0)).g;
    uvec2 B = uvec2(A.r, min(min(A.g, E + Beta), W + Beta));

    if (A == B || B.g > Infinity)
        discard;

    FragColor = B;
}

-- Fragment.GL3.Erosion.VerticalRed --

// Fragment.GL3.Erosion.VerticalRed \\

out uvec2 FragColor;
uniform usampler2D Sampler;
uniform uint Beta;
uniform uint Infinity;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);

    uvec2 A = texelFetch(Sampler, coord, 0).rg;
    uint E = texelFetchOffset(Sampler, coord, 0, ivec2(0, +1)).r;
    uint W = texelFetchOffset(Sampler, coord, 0, ivec2(0, -1)).r;
    uvec2 B = uvec2(min(min(A.r, E + Beta), W + Beta), A.g);

    if (A == B || B.r > Infinity)
        discard;

    FragColor = B;
}

-- Fragment.GL3.Erosion.VerticalGreen --

// Fragment.GL3.Erosion.VerticalGreen \\

out uvec2 FragColor;
uniform usampler2D Sampler;
uniform uint Beta;
uniform uint Infinity;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);

    uvec2 A = texelFetch(Sampler, coord, 0).rg;
    uint E = texelFetchOffset(Sampler, coord, 0, ivec2(0, +1)).g;
    uint W = texelFetchOffset(Sampler, coord, 0, ivec2(0, -1)).g;
    uvec2 B = uvec2(A.r, min(min(A.g, E + Beta), W + Beta));

    if (A == B || B.g > Infinity)
        discard;

    FragColor = B;
}

-------------------------------------------
 _____                         
/  ___|                        
\ `--.   ___  ___  _ __    ___ 
 `--. \ / __|/ _ \| '_ \  / _ \
/\__/ /| (__|  __/| | | ||  __/
\____/  \___|\___||_| |_| \___|
                               
-- Vertex.GL3.Scene --

// Vertex.GL3.Scene \\

in vec4 Position;
uniform mat4 Projection;
void main()
{
    gl_Position = Projection * Position;
}

-- Fragment.GL3.Scene --

out uvec2 FragColor;
uniform uvec2 FillColor;
void main()
{
    FragColor = FillColor;
}

-------------------------------------------
 _   _  _                     _  _           
| | | |(_)                   | |(_)          
| | | | _  ___  _   _   __ _ | | _  ____ ___ 
| | | || |/ __|| | | | / _` || || ||_  // _ \
\ \_/ /| |\__ \| |_| || (_| || || | / /|  __/
 \___/ |_||___/ \__,_| \__,_||_||_|/___|\___|

-- Vertex.GL3.Visualize --

// Vertex.GL3.Visualize \\

in vec4 Position;
void main()
{
    gl_Position = Position;
}

-- Fragment.GL3.Visualize --

// Fragment.GL3.Visualize \\

out vec4 FragColor;
uniform usampler2D Sampler;
uniform uint Infinity;

void main()
{
    ivec2 coord  = ivec2(gl_FragCoord.xy);
    uvec2 d = texelFetch(Sampler, coord, 0).xy;
    vec2 rg = vec2(d) / vec2(Infinity);
    FragColor = vec4(rg, 0, 1);
}

-- Fragment.GL3.Visualize.Sine --

// Fragment.GL3.Visualize.Sine \\

out vec4 FragColor;
uniform usampler2D Sampler;
uniform uint Infinity;

void main()
{
    ivec2 coord  = ivec2(gl_FragCoord.xy);
    uvec2 d = texelFetch(Sampler, coord, 0).xy;
    vec2 rg = vec2(d) / vec2(Infinity);
    rg = (1.0 - rg) * (1.0 - rg) * (0.5 + sin(rg * 3.14159 * 16) / 2.0);
    FragColor = vec4(rg, 0, 1);
}

-- Fragment.GL3.Visualize.Normalized --

// Fragment.GL3.Visualize.Normalized \\

out vec4 FragColor;
uniform sampler2D Sampler;

void main()
{
    ivec2 coord  = ivec2(gl_FragCoord.xy);
    float A = 1.0 - texelFetch(Sampler, coord, 0).r;
    FragColor = vec4(0, 0, 0, A);
}

-------------------------------------------
 _   _                                 _  _           
| \ | |                               | |(_)          
|  \| |  ___   _ __  _ __ ___    __ _ | | _  ____ ___ 
| . ` | / _ \ | '__|| '_ ` _ \  / _` || || ||_  // _ \
| |\  || (_) || |   | | | | | || (_| || || | / /|  __/
\_| \_/ \___/ |_|   |_| |_| |_| \__,_||_||_|/___|\___|
                                                      
-- Vertex.GL3.Normalize --

// Vertex.GL3.Normalize \\

in vec4 Position;
void main()
{
    gl_Position = Position;
}

-- Fragment.GL3.Normalize --

// Fragment.GL3.Normalize \\

out float FragColor;
uniform usampler2D Sampler;
uniform uint Infinity;
void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);
    uvec2 values = texelFetch(Sampler, coord, 0).rg;
    if (values.r > 0u)
    {
        FragColor = 0.5 + 0.5 * float(values.x) / float(Infinity);
    }
    else
    {
        FragColor = 0.5 - 0.5 * float(values.y) / float(Infinity);
    }
}
