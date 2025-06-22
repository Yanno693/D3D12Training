# Shader Test

function compileShader($shader)
{
    
    $shader_vs_src = ".\Shaders\" + $shader + "_vs.hlsl"
    $shader_vs_cso = ".\Shaders\" + $shader + "_vs.cso"
    $shader_ps_src = ".\Shaders\" + $shader + "_ps.hlsl"
    $shader_ps_cso = ".\Shaders\" + $shader + "_ps.cso"
    $shader_refl =   ".\Shaders\" + $shader + "_refl.cso"

    Write-Output "Compiling $shader ..."
    ./dxc.exe -Od -Zi -Qembed_debug -E main -T vs_6_3 -Fo $shader_vs_cso -Fre $shader_refl $shader_vs_src
    ./dxc.exe -Od -Zi -Qembed_debug -E main -T ps_6_3 -Fo $shader_ps_cso $shader_ps_src    
}

function compileShaderRT($shader)
{
    $shader_rg_src   = ".\Shaders\" + $shader + "_rg.hlsl"
    $shader_ch_src   = ".\Shaders\" + $shader + "_ch.hlsl"
    $shader_miss_src = ".\Shaders\" + $shader + "_miss.hlsl"

    $shader_rg_cso     = ".\Shaders\" + $shader + "_rg.cso"
    $shader_ch_cso     = ".\Shaders\" + $shader + "_ch.cso"
    $shader_miss_cso   = ".\Shaders\" + $shader + "_miss.cso"
    

    Write-Output "Compiling RT $shader ..."
    ./dxc.exe -Od -Zi -Qembed_debug -E mainRayGen -T lib_6_3 -Fo $shader_rg_cso $shader_rg_src
    ./dxc.exe -Od -Zi -Qembed_debug -E mainHit -T lib_6_3 -Fo $shader_ch_cso $shader_ch_src
    ./dxc.exe -Od -Zi -Qembed_debug -E mainMiss -T lib_6_3 -Fo $shader_miss_cso $shader_miss_src
}

function compileShaderRayGenRT($shader)
{
    $shader_rg_src   = ".\Shaders\" + $shader + "_rg.hlsl"

    $shader_rg_cso     = ".\Shaders\" + $shader + "_rg.cso"
    

    Write-Output "Compiling RT $shader ..."
    ./dxc.exe -Od -Zi -Qembed_debug -E mainRayGen -T lib_6_3 -Fo $shader_rg_cso $shader_rg_src
}

#./dxc.exe -Zi -Qembed_debug -E main -T vs_6_0 -Fo .\Shaders\test_vs.cso -Fre .\Shaders\test_refl.cso -Frs .\Shaders\test_rs.cso .\Shaders\test_vs.hlsl
#./dxc.exe -Zi -Qembed_debug -E main -T ps_6_0 -Fo .\Shaders\test_ps.cso .\Shaders\test_ps.hlsl

compileShader("test")
compileShader("basicsolid")
compileShaderRT("basicsolidrt")
compileShaderRayGenRT("default")