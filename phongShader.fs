#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D brickTexture;
uniform sampler2D normalMapTex;

uniform vec3 lightPositions[4];
uniform vec3 viewPos;
uniform bool blinn;
uniform float shininess;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;

// can add normal mapping if needed
 vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMapTex, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{           
    vec3 color = texture(brickTexture, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.2 * color;
    // diffuse
    //vec3 normal = getNormalFromMap();
    vec3 normal = normalize(fs_in.Normal);
    float totSpec = 0.0f;
    vec3 totDiff = vec3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < 4; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fs_in.FragPos);
        //vec3 normal = normalize(fs_in.Normal);
        // INCLUDE THIS FOR NORMAL MAPPING
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * color;
        totDiff = totDiff + diffuse;
        // specular
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        if(blinn)
        {
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        }
        else
        {
            vec3 reflectDir = reflect(-lightDir, normal);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        }
        totSpec = totSpec + spec;
    }
    vec3 specular = vec3(0.5) * totSpec * materialSpecular; // assuming bright white light color
    FragColor = vec4(ambient + totDiff*materialDiffuse + specular, 1.0);
}
