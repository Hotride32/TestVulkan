#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    int  light;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//layout(location = 3) in vec3 enable;

void main()
{
    vec4 color;
   // vec3 lightVector = vec3(0,0,1*ubo.light); 
    vec3 lightVector = vec3(0,0,1);//changed from (0,0,1)
    //*ubo.light
    float cosTheta = dot( fragNormal,lightVector ); //added +2
    vec4 baseColor = texture(texSampler, fragTexCoord);
    if (cosTheta > 0.98)
		outColor = baseColor + baseColor * 1.5;
	else if  (cosTheta  > 0.9)
		outColor = baseColor + baseColor * 1.0;
	else if (cosTheta  > 0.5)
		outColor = baseColor + baseColor * 0.6;
	else if (cosTheta  > 0.25)
		outColor = baseColor + baseColor * 0.4;
	else
		outColor = baseColor + baseColor * 0.2;
	// Desaturate a bit
	//color = vec4(mix(color, vec3(dot(vec3(0.2126,0.7152,0.0722), color)), 0.1));
    
    
    //outColor = baseColor + baseColor * cosTheta;
    //outColor.rgb = color; 
    outColor.w = baseColor.w;
}
