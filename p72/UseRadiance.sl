surface
    UseRadiance(uniform string filename = "";
                       color albedo = color(0.830, 0.791, 0.753);   // marble
  	               color dmfp = color(8.51, 5.57, 3.95);   // marble
                       float ior = 1.5;   // marble
		       float unitlength = 1.0;   // modeling scale
                       float smooth = 0;
		       float maxsolidangle = 1.0;   // quality knob: lower is better
                       float samples = 64;        
                       float Ka = 1, Kd = 0, Ks = 1, roughness = 0.1)
    {
      normal Nf = faceforward(normalize(N), I);
        normal Nn = normalize(Nf);
        vector V = -normalize(I);
        color amb, diff, spec, sss = 0;

        // Compute direct illumination (ambient, diffuse, and specular)
        amb = Ka * ambient();
        diff = Kd * diffuse(Nn);
        spec = Ks * specular(Nn, V, roughness);

        // Compute subsurface scattering color
        sss = subsurface(P, N, "filename", filename,
                         "albedo", albedo, "diffusemeanfreepath", dmfp,
                         "ior", ior, "unitlength", unitlength,
                         "smooth", smooth,
			 "maxsolidangle", maxsolidangle);

//        float occ = occlusion(P, Nn, samples, "maxdist", 100.0);

        // Set Ci and Oi
        Ci = (amb + diff + sss) * Cs + spec;
//        Ci *= (1 - occ);
        Ci *= Os;
        Oi = Os;
    }
