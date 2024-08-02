#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in ivec4 bones;
layout (location = 4) in vec4 weights;
layout (location = 5) in int skinning_type;
layout (location = 6) in vec3 center;
layout (location = 7) in vec3 r0;
layout (location = 8) in vec3 r1;
layout (location = 9) in vec3 morph_pos;

out vec3 fnormal;
out vec2 fuv;

uniform mat4 mvp;

layout(std140, binding = 0) uniform Buffer0{
	mat4 FinalTransform[1]; // Œã‚ÅŽ©“®‚ÅŠg’£‚³‚ê‚é
};

layout(std140, binding = 1) uniform Buffer1{
	vec4 BoneRotation[1];
};

mat3 quatToMat3(vec4 q) {
    float x2 = q.x + q.x;
	float y2 = q.y + q.y;
	float z2 = q.z + q.z;
	float xx = q.x * x2;
	float xy = q.x * y2;
	float xz = q.x * z2;
	float yy = q.y * y2;
	float yz = q.y * z2;
	float zz = q.z * z2;
	float wx = q.w * x2;
	float wy = q.w * y2;
	float wz = q.w * z2;
	return mat3(1.0 - (yy + zz), xy + wz, xz - wy,
				xy - wz, 1.0 - (xx + zz), yz + wx,
				xz + wy, yz - wx, 1.0 - (xx + yy));
}

vec4 slerp(vec4 q0, vec4 q1, float t) {
   float cosTheta = dot(q0, q1);
	if(cosTheta < 0){
		q1 = -q1;
		cosTheta = -cosTheta;
	}
	if (cosTheta > 0.9995) {
		return mix(q0, q1, t);
	}
	float theta = acos(cosTheta);
	return (q0 * sin((1.0 - t) * theta) + q1 * sin(t * theta)) / sin(theta);
}

void main(){
	
	fnormal = normal;
	fuv = uv;

	mat4 skinned = mat4(1);
	if(skinning_type == 0){
		skinned = FinalTransform[bones[0]];
	}
	else if(skinning_type == 1){
		skinned = FinalTransform[bones[0]] * weights[0] +
				  FinalTransform[bones[1]] * weights[1] ;
	}
	else if(skinning_type == 2){
		skinned = FinalTransform[bones[0]] * weights[0] +
				  FinalTransform[bones[1]] * weights[1] +
				  FinalTransform[bones[2]] * weights[2] +
				  FinalTransform[bones[3]] * weights[3] ;
	}
	else if(skinning_type == 3){
		vec4 quat = slerp( BoneRotation[bones[0]], BoneRotation[bones[1]], weights[1] ); 
		mat3 rot_mat = quatToMat3(quat);
		vec3 skinned_pos = rot_mat * (position - center) + 
				vec3(FinalTransform[bones[0]] * vec4(r0,1))*weights[0] +
				vec3(FinalTransform[bones[1]] * vec4(r1,1))*weights[1];
		gl_Position = mvp * vec4(skinned_pos,1);
		return;
	}

	gl_Position = mvp * skinned * vec4(position + morph_pos, 1);
}