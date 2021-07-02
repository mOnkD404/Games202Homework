class PRTMaterial extends Material {

    constructor(color, specular, light, translate, scale, vertexShader, fragmentShader) {
        let lightIntensity = light.mat.GetIntensity();

        super({
            // Phong
            'uSampler': { type: 'texture', value: color },
            'uKs': { type: '3fv', value: specular },
            'uLightRadiance': { type: '3fv', value: lightIntensity },
            // PRT                   
            'uPrecomputeLR': { type: 'updatedInRealTime', value: null },
            'uPrecomputeLG': { type: 'updatedInRealTime', value: null },
            'uPrecomputeLB': { type: 'updatedInRealTime', value: null }
        }, ['aPrecomputeLT'], vertexShader, fragmentShader, null);
    }
}

async function buildPRTMaterial(color, specular, light, translate, scale, vertexPath, fragmentPath) {


    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new PRTMaterial(color, specular, light, translate, scale, vertexShader, fragmentShader);

}