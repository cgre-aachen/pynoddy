
.. code:: python

    # Rayleigh formula for free thermal convection, formulae taken from Kuhn and Gessner 2009, Surveys in Geophysics
    
    def Rayleigh():
        """ Properties of pure water at 20 degrees K """
        # values of pure water taken from http://people.ucsc.edu/~bkdaniel/WaterProperties.html
        alpha =((0.207 + 0.385 + 0.523) / 3) * 1*10**-3 # thermal expansivity of water
        print alpha
        rho = 998.0 # Density of pure water
        c = 4179.0 # Fluid heat capacity
        g = 9.812
        mu = ((1.004 + 0.658 + 0.475) / 3) * 1*10**-3 #dynamic viscosity of water
        """ Mean Values """    
        WLFM0_xyz = 2.7, #np.array(S1_out.get_array_as_xyz_structure("WLFM0"))
        WLFM0_mean = 2.7 #np.mean(WLFM0_xyz[:, :, :]) # Mean of medium's thermal expansivity
        #print WLFM0_mean
        
        perm_xyz = 1E-12 # np.array(S1_out.get_array_as_xyz_structure("PERM"))
        mean_perm = 1E-12 # np.mean(perm_xyz[:,:,:]) # Mean of permeability field
        #mean_perm = 5.835e-12
        #print "%0.4g" %mean_perm
        
       #temp_xyz = np.array(S1_out.get_array_as_xyz_structure("TEMP")) # determine temperature gradient at the subplot
        T2 = 170. #temp_xyz[0:, :, 0]
        T1 = 20. # temp_xyz[0:, :, -1]
        # z_axis = temp_xyz[0, :, 0:]
        # z_axis_list = list(z_axis[:,:].ravel(order = "F"))
        length = 1000. # len(z_axis_list) * 100.0
        #print length
        Value = (T2 - T1) / length
        dtemp_value = Value # np.mean(Value[:, :])
        
        print dtemp_value
        
        Ra = ((T2 - T1) * mean_perm * length * alpha * (rho**2) * g * c) / (WLFM0_mean * mu)
        return Ra
    print "%0.4g" %Rayleigh()


.. parsed-literal::

    0.000371666666667
    0.15
    1184


