attribute vec3 aVertexPosition;
attribute vec3 aNormalPosition;
attribute vec2 aTextureCoord;

attribute mat3 aPrecomputeLT;

uniform mat3 uPrecomputeLR;
uniform mat3 uPrecomputeLG;
uniform mat3 uPrecomputeLB;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;

varying highp vec3 vColor;

void main() {
    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix *
        vec4(aVertexPosition, 1.0);
    
    vNormal = (uModelMatrix * vec4(aNormalPosition, 0.0)).xyz;
    vFragPos = (uModelMatrix * vec4(aVertexPosition, 1.0)).xyz;
    vTextureCoord = aTextureCoord;


    vColor =vec3(
    dot(uPrecomputeLR[0], aPrecomputeLT[0])+ dot(uPrecomputeLR[1], aPrecomputeLT[1]) + dot(uPrecomputeLR[2], aPrecomputeLT[2]),
    dot(uPrecomputeLG[0], aPrecomputeLT[0])+ dot(uPrecomputeLG[1], aPrecomputeLT[1]) + dot(uPrecomputeLG[2], aPrecomputeLT[2]),
    dot(uPrecomputeLB[0], aPrecomputeLT[0])+ dot(uPrecomputeLB[1], aPrecomputeLT[1]) + dot(uPrecomputeLB[2], aPrecomputeLT[2])
    );

}