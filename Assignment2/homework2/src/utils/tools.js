function getRotationPrecomputeL(precompute_L, rotationMatrix){
    // mat4生成的矩阵，是纵向记录的，需要转置
	let rotation = math.transpose(mat4Matrix2mathMatrix(rotationMatrix));

	let matrix33 = computeSquareMatrix_3by3(rotation);
	let matrix55 = computeSquareMatrix_5by5(rotation);

	let precompute_rgb = math.transpose(precompute_L);
	let rotated_rgb = []

	for (let channel in precompute_rgb) {
		let precompute_color = math.clone(precompute_rgb[channel]);
		// l = 1
		let sh1 = [precompute_color[1], precompute_color[2], precompute_color[3]];
		let sh1m = math.multiply(matrix33, sh1);
		precompute_color[1] = sh1m[0];
		precompute_color[2] = sh1m[1];
		precompute_color[3] = sh1m[2];	

		// l = 2
		let sh2 = [precompute_color[4], precompute_color[5], precompute_color[6], precompute_color[7], precompute_color[8]];
		let sh2m = math.multiply(matrix55, sh2);
		precompute_color[4] = sh2m[0];
		precompute_color[5] = sh2m[1];
		precompute_color[6] = sh2m[2];
		precompute_color[7] = sh2m[3];
		precompute_color[8] = sh2m[4];
		rotated_rgb.push(precompute_color);
	}

    let result = getMat3ValueFromRGB(math.transpose(rotated_rgb));
	return result;
}

function computeSquareMatrix_3by3(rotationMatrix){ // 计算方阵SA(-1) 3*3 
	
	// 1、pick ni - {ni}
	let n1 = [1, 0, 0, 0]; let n2 = [0, 0, 1, 0]; let n3 = [0, 1, 0, 0];

	// 2、{P(ni)} - A  A_inverse
	let order = 3;
	let nish = [];
	let niv = [n1, n2, n3];
	for(let i = 0; i < order; i++) {
		let sh = SHEval3(niv[i][0], niv[i][1], niv[i][2]); // 1 * 9
		nish[i] = sh.slice(1,4);
	}
    let A = math.transpose(nish);	// 3 * 3
	let A_inv = math.inv(A);        // 3 * 3

	// 3、用 R 旋转 ni - {R(ni)}
	let nirv = math.multiply(rotationMatrix, math.transpose(niv)).toArray();// 4 * 3

	// 4、R(ni) SH投影 - S
	let nirsh = []; // 3 * 9
	for(let i = 0; i < order; i++) {
		nirsh[i] = SHEval3(nirv[0][i], nirv[1][i], nirv[2][i]).slice(1,4); // 1 * 3
	}
	let S = math.transpose(nirsh); // 3 * 3

	// 5、S*A_inverse
    return math.multiply(S, A_inv); // 3 * 3
}

function computeSquareMatrix_5by5(rotationMatrix){ // 计算方阵SA(-1) 5*5
	
	// 1、pick ni - {ni}
	let k = 1 / math.sqrt(2);
	let n1 = [1, 0, 0, 0]; let n2 = [0, 0, 1, 0]; let n3 = [k, k, 0, 0]; 
	let n4 = [k, 0, k, 0]; let n5 = [0, k, k, 0];

	// 2、{P(ni)} - A  A_inverse
	let order = 5;
	let nish = [];
	let niv = [n1, n2, n3, n4, n5];
	for(let i = 0; i < order; i++) {
		nish[i] = SHEval3(niv[i][0], niv[i][1], niv[i][2]).slice(4); // 1 * 5
	}
    let A = math.transpose(nish);	// 5 * 5
	let A_inv = math.inv(A);        // 5 * 5

	// 3、用 R 旋转 ni - {R(ni)}
	let nirv = math.multiply(rotationMatrix, math.transpose(niv)).toArray();// 4 * 5

	// 4、R(ni) SH投影 - S
	let nirsh = []; // 5 * 9
	for(let i = 0; i < order; i++) {
		nirsh[i] = SHEval3(nirv[0][i], nirv[1][i], nirv[2][i]).slice(4); // 1 * 5
	}
	let S = math.transpose(nirsh); // 5 * 5

	// 5、S*A_inverse
    return math.multiply(S, A_inv); // 5 * 5
}

function mat4Matrix2mathMatrix(rotationMatrix){

	let mathMatrix = [];
	for(let i = 0; i < 4; i++){
		let r = [];
		for(let j = 0; j < 4; j++){
			r.push(rotationMatrix[i*4+j]);
		}
		mathMatrix.push(r);
	}
	 return math.matrix(mathMatrix);
}

function getMat3ValueFromRGB(precomputeL){

    let colorMat3 = [];
    for(var i = 0; i<3; i++){
        colorMat3[i] = mat3.fromValues( precomputeL[0][i], precomputeL[1][i], precomputeL[2][i],
										precomputeL[3][i], precomputeL[4][i], precomputeL[5][i],
										precomputeL[6][i], precomputeL[7][i], precomputeL[8][i] ); 
	}
    return colorMat3;
}