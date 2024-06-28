cbuffer ConstantBuffer : register(b0)
{
    float4 color1;
    float4 color2;
    float4 color3;
    float2 dim;
    float time;
    float aspectRatio;
    float rad;
    float maxLen;
    float movementTimeScale;
    float waveTimeScale;
    float deadzoneFloor;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT VS(float4 pos : POSITION, float2 uv : TEXCOORD0)
{
    VS_OUTPUT output;
    output.pos = pos;
    output.uv = uv;
    return output;
}

// ======= PIXEL =======

float clampToRange(float value, float maxValue)
{
    maxValue = abs(maxValue);
    value = clamp(value, -maxValue, maxValue);
    return value / maxValue;
}

float2 gradientRandom(float2 p)
{
    return frac(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453);
}

float gradientNoise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    float2 u = f * f * (3.0 - 2.0 * f);

    float final = lerp(
        lerp(dot(gradientRandom(i + float2(0.0, 0.0)), f - float2(0.0, 0.0)),
             dot(gradientRandom(i + float2(1.0, 0.0)), f - float2(1.0, 0.0)), u.x),
        lerp(dot(gradientRandom(i + float2(0.0, 1.0)), f - float2(0.0, 1.0)),
             dot(gradientRandom(i + float2(1.0, 1.0)), f - float2(1.0, 1.0)), u.x), u.y);
    
    return final;
}

float isPointInBox(float2 p, float2 boxStart, float2 boxLength, float boxWidth)
{
    float2 boxDirection = normalize(boxLength);
    float2 boxPerpendicular = float2(-boxDirection.y, boxDirection.x);
    float2 relativePosition = p - boxStart;
    
    float projectionLength = dot(relativePosition, boxDirection);
    float projectionWidth = dot(relativePosition, boxPerpendicular);
    
    bool isWithinLength = (projectionLength >= 0) && (projectionLength <= length(boxLength));
    bool isWithinWidth = abs(projectionWidth) <= boxWidth * 0.5;
    float widthVal = smoothstep(boxWidth * 0.5, boxWidth * 0.5 - 0.001, abs(projectionWidth));

    float final = 0;
    if (isWithinLength && isWithinWidth)
    {
        final = projectionLength / length(boxLength);
    }
    return final * widthVal;
}

float checkCircle(float2 p, float2 boxStart, float2 boxLength, float radius)
{
    float2 center = boxStart + boxLength;
    float val = clamp(smoothstep(radius, radius - 0.001, length(p - center)), 0, 1);
    return val;
}

float Drift(float2 uv, float2 dimensions, float radius, float maxlength, float2 scrollX, float2 scrollY, float aspect)
{
    uv.x *= aspect;
    
    float Out = 0;
    float2 minPoint = max(uv - maxlength - radius * 2, 0);
    float2 maxPoint = min(uv + maxlength + radius * 2, dimensions);
    int2 minIndex = int2(floor(minPoint * dimensions));
    int2 maxIndex = int2(ceil(maxPoint * dimensions));

    for (int y = minIndex.y; y <= maxIndex.y; y++)
    {
        for (int x = minIndex.x; x <= maxIndex.x; x++)
        {
            float2 pnt = { (float) x - 0.5, (float) y - 0.5 };
            pnt /= dimensions;
            float2 disp = { clampToRange(gradientNoise(pnt + scrollX), 0.5f), clampToRange(gradientNoise(pnt * 2 + 10 + scrollY), 0.5f) };
            disp *= maxlength;
            float circleVal = checkCircle(uv, pnt, disp, radius);
            Out = max(circleVal, Out);
            
            if (length(uv - pnt) <= length(disp))
            {
                Out = max(isPointInBox(uv, pnt, disp, radius * 2), Out);
            }
        }
    }
    return Out;
}


float4 PS(VS_OUTPUT input) : SV_Target
{
    float2 scrollX = { time * movementTimeScale, 0 };
    float2 scrollY = { 0, time * movementTimeScale };
    
    float mask = Drift(input.uv, dim, rad, maxLen, scrollX, scrollY, aspectRatio);
    float4 final = float4(mask, mask, mask, 1);
    
    float waveTime = waveTimeScale * time;
    
    float4 color1Noise = abs(gradientNoise(input.uv + float2(waveTime, waveTime)) * 4) * color1;
    float4 color2Noise = abs(gradientNoise(input.uv + float2(10, 0) + float2(waveTime, 0)) * 4) * color2;
    float4 color3Noise = abs(gradientNoise(input.uv + float2(20, 0) + float2(0, waveTime)) * 4) * color3;
    
    float4 colorFinal = saturate(color1Noise + color2Noise + color3Noise);
    
    float deadzone = abs(gradientNoise(input.uv * 1.5 + float2(0, 2) + float2(waveTime, 0)) * 4)
        + abs(gradientNoise(input.uv * 1.5 + float2(2, 0) + float2(0, waveTime)) * 4);
    deadzone = smoothstep(deadzoneFloor, 1, deadzone);
    
    final *= colorFinal * deadzone;
    
    return final;
}