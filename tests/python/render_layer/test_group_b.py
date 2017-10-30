# ############################################################
# Importing - Same For All Render Layer Tests
# ############################################################

import unittest
import os
import sys

from render_layer_common import *


# ############################################################
# Testing
# ############################################################

class UnitTesting(RenderLayerTesting):
    def test_group_create_basic(self):
        """
        See if the creation of new groups is working
        """
        import bpy
        scene = bpy.context.scene

        # clean slate
        self.cleanup_tree()

        master_collection = scene.master_collection
        grandma = master_collection.collections.new('бабушка')
        mom = grandma.collections.new('матушка')

        child = bpy.data.objects.new("Child", None)
        mom.objects.link(child)

        grandma_layer_collection = scene.render_layers[0].collections.link(grandma)
        mom_layer_collection = grandma_layer_collection.collections[0]

        grandma_layer_collection.hide = False
        grandma_layer_collection.hide = False
        mom_layer_collection.hide = True
        mom_layer_collection.hide_select = False

        # update depsgraph
        scene.update()

        # create group
        group = grandma_layer_collection.create_group()

        # update depsgraph
        scene.update()

        # compare
        self.assertEqual(len(group.collections), 1)
        grandma_group_layer = group.collections[0]

        self.assertEqual(grandma_group_layer.hide, False)
        self.assertEqual(grandma_group_layer.hide_select, False)

        self.assertEqual(len(grandma_group_layer.collections), 1)
        mom_group_layer = grandma_group_layer.collections[0]

        self.assertEqual(mom_group_layer.hide, True)
        self.assertEqual(mom_group_layer.hide_select, False)


# ############################################################
# Main - Same For All Render Layer Tests
# ############################################################

if __name__ == '__main__':
    UnitTesting._extra_arguments = setup_extra_arguments(__file__)
    unittest.main()
