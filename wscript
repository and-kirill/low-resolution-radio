## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('low-resolution-radio', ['core', 'network', 'internet', 'propagation', 'spectrum', 'mobility'])

    module.source = [
      'model/lrr-channel.cc',
      'model/lrr-range-error-model.cc',
      'model/lrr-phy.cc',
      'model/lrr-device.cc',
      'model/lrr-device-impl.cc',
      'model/lrr-mac-access-manager.cc',
      'model/lrr-mac.cc',
      'model/lrr-mac-impl.cc',
      'model/lrr-mac-header.cc',
      'model/lrr-mcast-group-mgt.cc',
      'model/lrr-mcast-table.cc',
      'model/lrr-routing-dpd.cc',
      'model/lrr-routing-graph.cc',
      'model/lrr-routing-topology.cc',
      'model/lrr-routing-peering.cc',
      'model/lrr-routing-protocol.cc',
      'model/lrr-routing-seq-cache.cc',
      'helper/lrr-routing-helper.cc',
      'helper/lrr-device-helper.cc',
      'helper/lrr-channel-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('low-resolution-radio')

    module_test.source = [
      'test/lrr-routing-graph-test.cc',
      'test/lrr-routing-mcast-test.cc',
      'test/lrr-group-mgt-test.cc',
             ]

    headers = bld(features='ns3header')
    headers.module = 'low-resolution-radio'

    # Set the C++ header files for this module.
    headers.source = [
      'model/lrr-channel.h',
      'model/lrr-range-error-model.h',
      'model/lrr-phy.h',
      'model/lrr-device.h',
      'model/lrr-device-impl.h',
      'model/lrr-mac-access-manager.h',
      'model/lrr-mac.h',
      'model/lrr-mac-impl.h',
      'model/lrr-mac-header.h',
      'model/lrr-mcast-group-mgt.h',
      'model/lrr-routing-dpd.h',
      'model/lrr-routing-graph.h',
      'model/lrr-routing-topology.h',
      'model/lrr-routing-peering.h',
      'model/lrr-routing-protocol.h',
      'model/lrr-routing-seq-cache.h',
      'helper/lrr-routing-helper.h',
      'helper/lrr-device-helper.h',
      'helper/lrr-channel-helper.h',
        ]

    if (bld.env['ENABLE_EXAMPLES']):
        bld.recurse('examples')


