<?php

// ------------------------------------------------
// @(#) dfsiexport.permission.inc
// @(#) Permissions
// @(#) Amael Hervoche
// @(#) 24/02/2014
// ------------------------------------------------


// --------------------------------
// Hook_permission
// --------------------------------
// créee les permissions 
// specifiques au module dfsiexport
// --------------------------------
function dfsiexport_permission() 
{
    return array
    (
        'export dfsi data' => array
        (
			'title' => t('exporter les données de ')._MODULE_COMMERCIAL_NAME,
			'description' => t("permission d'exportation")
        ),
    );
}