#version 330 core
out vec4 FragColor;

in vec3 fragPos;

uniform float timeOfDay; // 0.0 = night, 0.25 = sunrise, 0.5 = day, 0.75 = sunset, 1.0 = midnight

float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

vec3 getSkyColor(vec3 direction, float tod) {
    float height = direction.y;
    
    // Sun direction changes with time of day (rotates around)
    float sunAngle = tod * 3.14159 * 2.0;
    vec3 sunDir = normalize(vec3(cos(sunAngle), sin(sunAngle) * 0.5, sin(sunAngle)));
    
    float sunDot = max(dot(direction, sunDir), 0.0);
    
    // Define colors for different times of day
    vec3 nightSkyTop = vec3(0.01, 0.01, 0.05);
    vec3 nightSkyHorizon = vec3(0.05, 0.05, 0.15);
    
    vec3 daySkyTop = vec3(0.3, 0.5, 0.9);
    vec3 daySkyHorizon = vec3(0.6, 0.7, 0.9);
    
    vec3 sunriseSkyTop = vec3(0.4, 0.3, 0.5);
    vec3 sunriseSkyHorizon = vec3(1.0, 0.5, 0.3);
    
    vec3 sunsetSkyTop = vec3(0.5, 0.3, 0.4);
    vec3 sunsetSkyHorizon = vec3(1.0, 0.4, 0.2);
    
    // Smooth interpolation between times
    vec3 skyTop, skyHorizon;
    
    if (tod < 0.25) {
        // Night to sunrise
        float t = tod / 0.25;
        skyTop = mix(nightSkyTop, sunriseSkyTop, t);
        skyHorizon = mix(nightSkyHorizon, sunriseSkyHorizon, t);
    } else if (tod < 0.5) {
        // Sunrise to day
        float t = (tod - 0.25) / 0.25;
        skyTop = mix(sunriseSkyTop, daySkyTop, t);
        skyHorizon = mix(sunriseSkyHorizon, daySkyHorizon, t);
    } else if (tod < 0.75) {
        // Day to sunset
        float t = (tod - 0.5) / 0.25;
        skyTop = mix(daySkyTop, sunsetSkyTop, t);
        skyHorizon = mix(daySkyHorizon, sunsetSkyHorizon, t);
    } else {
        // Sunset to midnight
        float t = (tod - 0.75) / 0.25;
        skyTop = mix(sunsetSkyTop, nightSkyTop, t);
        skyHorizon = mix(sunsetSkyHorizon, nightSkyHorizon, t);
    }
    
    // Blend between horizon and top based on height
    float horizonBlend = smoothstep(-0.3, 0.3, height);
    vec3 skyColor = mix(skyHorizon, skyTop, horizonBlend);
    
    // Add sun glow
    float sunIntensity = pow(sunDot, 128.0) * 2.0;
    vec3 sunColor = vec3(1.0, 0.95, 0.8);
    
    // Sun disc
    float sunDisc = smoothstep(0.9995, 0.9998, sunDot);
    
    // Atmospheric glow around sun
    float atmosphereGlow = pow(sunDot, 8.0) * 0.5;
    
    // Only show sun during day/sunrise/sunset
    float sunVisibility = 1.0;
    if (tod < 0.1 || tod > 0.9) {
        sunVisibility = 0.0;
    } else if (tod < 0.2) {
        sunVisibility = (tod - 0.1) / 0.1;
    } else if (tod > 0.8) {
        sunVisibility = (0.9 - tod) / 0.1;
    }
    
    skyColor += sunColor * (sunDisc + atmosphereGlow) * sunVisibility;
    skyColor += sunColor * sunIntensity * 0.3 * sunVisibility;
    
    if (tod < 0.2 || tod > 0.8) {
        float starIntensity = 1.0;
        if (tod < 0.2) {
            starIntensity = (0.2 - tod) / 0.2;
        } else if (tod > 0.8) {
            starIntensity = (tod - 0.8) / 0.2;
        }
        
        // Use normalized direction for stable star positions
        vec3 starPos = normalize(direction) * 100.0;
        float starNoise = hash(floor(starPos));
        
        // Create distinct bright stars
        float stars = smoothstep(0.996, 1.0, starNoise) * starIntensity;
        
        // Add some variation in brightness
        float brightness = hash(floor(starPos * 1.3)) * 0.5 + 0.5;
        stars *= brightness;
        
        skyColor += vec3(stars);
    }
    
    return skyColor;
}

void main()
{    
    vec3 direction = normalize(fragPos);
    vec3 color = getSkyColor(direction, timeOfDay);
    FragColor = vec4(color, 1.0);
}
