{
  'variables': {
    'use_aura%': 1,
  },
  'targets': [
  {
    'target_name': 'blinq',
    'type': 'none',
    'dependencies': [
      'process/process.gyp:*',
      'lib/lib.gyp:*',
    ],
  }
  ]
}
