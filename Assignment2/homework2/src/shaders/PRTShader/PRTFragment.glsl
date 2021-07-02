#ifdef GL_ES
precision mediump float;
#endif
// Phong related variables
uniform sampler2D uSampler;
uniform vec3 uKd;
uniform vec3 uKs;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;

varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;

varying highp vec3 vColor;

vec3 blinnPhong() {
  vec3 color = texture2D(uSampler, vTextureCoord).rgb;
  color = pow(color, vec3(2.2));

//   vec3 ambient = 0.05 * color;
  vec3 ambient = vColor;

  vec3 lightDir = normalize(uLightPos);
  vec3 normal = normalize(vNormal);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * uLightRadiance * color;

  vec3 viewDir = normalize(uCameraPos - vFragPos);
  vec3 halfDir = normalize((lightDir + viewDir));
  float spec = pow(max(dot(halfDir, normal), 0.0), 32.0);
  vec3 specular = uKs * uLightRadiance * spec;

  vec3 radiance = (ambient + diffuse + specular);
  vec3 phongColor = pow(radiance, vec3(1.0 / 2.2));
  return phongColor;
}

void  main(void){
 //gl_FragColor = vec4(0.0,.75,1.0, 0.3);
 //gl_FragColor = vec4(1.0,1.0,1.0, 1.0);

 gl_FragColor = vec4(vColor, 1.0);

//   vec3 phongColor = blinnPhong();
//   gl_FragColor = vec4(phongColor, 1.0);
}