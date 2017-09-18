#version 450
#extension GL_ARB_tessellation_shader : enable

layout(triangles) in;
layout(location = 0) in vec3 tcCartesianPosition[];
layout(location = 1) in vec3 tcColor[];

layout(location = 0) out vec3 teCartesianPosition;
layout(location = 1) out vec3 teColor;

void main()
{
  // scalar products of vertex pairs -- to get angles and everything else
  float psps = dot(tcCartesianPosition[0], tcCartesianPosition[0]),
        pspt = dot(tcCartesianPosition[0], tcCartesianPosition[1]),
        ptpt = dot(tcCartesianPosition[1], tcCartesianPosition[1]),
        pupu = dot(tcCartesianPosition[2], tcCartesianPosition[2]);

  // angles
  float ast = gl_TessCoord.y / ( gl_TessCoord.x + gl_TessCoord.y )
            * acos( pspt / sqrt (psps * ptpt) ),
        cosAst = cos(ast),
        sinAst = sqrt(1 - cosAst*cosAst);

  // first barycentric coefficient
  float kst = gl_TessCoord.y <= 0
            ? 0
            : ( gl_TessCoord.x <= 0
              ? 1
              : clamp
                 ( psps * sinAst /
                     ( (psps - pspt ) * sinAst
                     + sqrt(psps*ptpt - pspt*pspt) * cosAst
                     )
                 , 0.0f, 1.0f)
              );

  // temporary vector -- a combination of ps and pt
  vec3 pp = kst * tcCartesianPosition[1] + (1-kst)*tcCartesianPosition[0];

  // scalar products for the new vector and pu
  float pupp = dot(tcCartesianPosition[2], pp),
        pppp = dot(pp,pp);

  // again, the second angle
  float aup = gl_TessCoord.z * acos( pupp / sqrt (pppp * pupu) ),
        cosAup = cos(aup),
        sinAup = sqrt(1 - cosAup*cosAup);

  // all barycentric coefficients
  float u = clamp
               ( pppp * sinAup /
                   ( (pppp - pupp ) * sinAup
                   + sqrt(pupu*pppp - pupp*pupp) * cosAup
                   )
               , 0.0f, 1.0f),
        t = (1-u)*kst,
        s = (1-u)*(1-kst);

  // interpolated cartesian position
  teCartesianPosition
    = s * tcCartesianPosition[0]
    + t * tcCartesianPosition[1]
    + u * tcCartesianPosition[2];

  // interpolated color
  teColor
    = s * tcColor[0]
    + t * tcColor[1]
    + u * tcColor[2];
}
