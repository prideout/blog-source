    surface
    BakeRadiance(string filename = "", displaychannels = ""; color Kdt = 1)
    {
        color irrad, rad_t;
        normal Nn = normalize(N);
        float a = area(P, "dicing"); // micropolygon area

        /* Compute direct illumination (ambient and diffuse) */
        irrad = ambient() + diffuse(Nn);

        /* Compute the radiance diffusely transmitted through the surface.
           Kdt is the surface color (could use a texture for e.g. freckles) */
        rad_t = Kdt * irrad;

        /* Store in point cloud file */
        bake3d(filename, displaychannels, P, Nn, "interpolate", 1,
               "_area", a, "_radiance_t", rad_t);

        Ci = rad_t * Cs * Os;
        Oi = Os;
    }