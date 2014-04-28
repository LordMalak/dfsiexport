<?php

// ------------------------------------------------
// @(#) dfsiexport.api.inc
// @(#) Accesseurs wrappers
// @(#) Amael Hervoche
// @(#) 05/03/2014
// ------------------------------------------------


// ---------------------------
// Acesseur du statut d'un noeud
// ---------------------------
function dfsiexport_GetStatutsOf( $node )
{
    $wrapper = entity_metadata_wrapper('node', $node);
    return $wrapper-> field_statut->value();    
}
// ----------------------------
// Acesseur de la descrition d'un noeud
// ---------------------------
function dfsiexport_GetBodyOf( $node )
{
	$wrapper = entity_metadata_wrapper('node', $node);
	$wrapper = $node->body['und'][0]['value'];
	return $wrapper;
}
// ----------------------------
// Acesseur de la criticite d'un noeud
// ---------------------------
function dfsiexport_GetCriticiteOf( $node )
{
    $wrapper = entity_metadata_wrapper('node', $node);
	return $wrapper-> field_criticite->value();  
}