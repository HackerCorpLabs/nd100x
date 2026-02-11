// sintran-seg-names.js - System segment name lookup table
// Source: sintran-system-segments.json (SINTRAN III Release Information)
// System segments 2-93 (002-135 octal)
// User segments (136-255) have no system names - identified by RT cross-reference only.
// Segment 1 (5BCOM) and 65 (SDNAM) are NPL symbols only, not SEGFIL names.

(function() {
  'use strict';

  // Segment number (decimal) -> { name, description, category }
  var SYSTEM_SEGMENTS = {
    1:   { name: null,         desc: 'Base common code (NPL symbol 5BCOM, not in SEGFIL table)', cat: 'kernel' },
    2:   { name: 'S3IMAGE',   desc: 'Image of common code, start/restart', cat: 'image' },
    3:   { name: 'S3CP',      desc: 'Command processor - SINTRAN command interpretation', cat: 'active' },
    4:   { name: 'S3RTL',     desc: 'RT-Loader - loads and manages RT programs', cat: 'active' },
    5:   { name: 'S3ERRS',    desc: 'Error/PIT segment', cat: 'kernel' },
    6:   { name: 'S3FS',      desc: 'File system - disk I/O and file management', cat: 'active' },
    7:   { name: 'S3DMAC',    desc: 'DMAC (Direct Memory Access Controller)', cat: 'active' },
    8:   { name: 'S3RTFIL',   desc: 'Runtime file operations', cat: 'active' },
    9:   { name: 'S3ERRL',    desc: 'Error log - system error records', cat: 'data' },
    10:  { name: 'S3SFS',     desc: 'Save of file system', cat: 'save' },
    11:  { name: 'S3SCP',     desc: 'Save of command processor', cat: 'save' },
    12:  { name: 'S3ERRP',    desc: 'Error program - error handling code', cat: 'active' },
    13:  { name: 'S3BFLY',    desc: 'Reserved for Butterfly (unused)', cat: 'reserved' },
    14:  { name: 'S3SRPIT',   desc: 'Save of RPIT', cat: 'save' },
    15:  { name: 'S3SMPIT',   desc: 'Save of MPIT', cat: 'save' },
    16:  { name: 'S3SDT5',    desc: 'ND-500/5000 standard domains', cat: 'nd500' },
    17:  { name: 'S3NM5',     desc: 'ND-500/5000 name-tables', cat: 'nd500' },
    18:  { name: 'S3RFAC',    desc: 'Remote file access (FIU)', cat: 'active' },
    19:  { name: 'S3DPIT',    desc: 'DPIT - Data Page Index Table, maps kernel data', cat: 'kernel' },
    20:  { name: 'S3SGST',    desc: 'Save of segment table', cat: 'save' },
    21:  { name: 'S3IRPIT',   desc: 'Image of RPIT', cat: 'image' },
    22:  { name: 'S3IMPIT',   desc: 'Image of MPIT', cat: 'image' },
    23:  { name: 'S3ISGT',    desc: 'Image of segment table', cat: 'image' },
    24:  { name: 'S3SM5',     desc: 'ND-500/5000 System Monitor', cat: 'nd500' },
    25:  { name: 'S3SSPD',    desc: 'Save of spooling data fields', cat: 'save' },
    29:  { name: 'S3MPIT',    desc: 'MPIT - Monitor Page Index Table, maps monitor code', cat: 'kernel' },
    30:  { name: 'S3TAD',     desc: 'Terminal administration', cat: 'active' },
    31:  { name: 'S3RTD',     desc: 'RT-Loader data', cat: 'data' },
    32:  { name: 'S3FUDRT',   desc: 'File user data for RT programs', cat: 'data' },
    33:  { name: 'S3IMED',    desc: 'Image of edit routines', cat: 'image' },
    34:  { name: 'S3ED',      desc: 'Edit routines - command line editor', cat: 'active' },
    35:  { name: 'S3PATCH',   desc: 'Live system patches', cat: 'system' },
    36:  { name: 'S3IDPIT',   desc: 'Image of DPIT', cat: 'image' },
    37:  { name: 'S3ISYS',    desc: 'Image of system segment', cat: 'image' },
    38:  { name: 'S3S5PIT',   desc: 'Save of 5PIT', cat: 'save' },
    39:  { name: 'S3RPIT',    desc: 'RPIT - Runtime PIT, maps RT program code', cat: 'kernel' },
    40:  { name: 'S3IS5PIT',  desc: 'Image of 5PIT', cat: 'image' },
    41:  { name: 'S35PIT',    desc: '5PIT - secondary PIT for ND-500 operations', cat: 'kernel' },
    42:  { name: 'S3SAVE',    desc: 'Save of common code and start/restart', cat: 'save' },
    43:  { name: 'S3SDPIT',   desc: 'Save of DPIT', cat: 'save' },
    44:  { name: 'S3SSYS',    desc: 'Save of system segment', cat: 'save' },
    45:  { name: 'S3SERRP',   desc: 'Save of error program', cat: 'save' },
    46:  { name: 'S3SRTC',    desc: 'Save of RT-Loader code', cat: 'save' },
    47:  { name: 'S3SRTD',    desc: 'Save of RT-Loader data', cat: 'save' },
    48:  { name: 'S3SECOM',   desc: 'Save of extended common', cat: 'save' },
    49:  { name: 'S3IECOM',   desc: 'Image of extended common', cat: 'image' },
    50:  { name: 'S3SSM5',    desc: 'Save of ND-500 System Monitor', cat: 'save' },
    51:  { name: 'S3MEMTF',   desc: 'Memory configuration flag', cat: 'system' },
    52:  { name: 'S3ECOM',    desc: 'Extended common - additional shared kernel code', cat: 'kernel' },
    53:  { name: 'S3SIPIT',   desc: 'Save of IPIT', cat: 'save' },
    54:  { name: 'S3IIPIT',   desc: 'Image of IPIT', cat: 'image' },
    55:  { name: 'S3IPIT',    desc: 'IPIT - Interrupt PIT, maps I/O and interrupt code', cat: 'kernel' },
    56:  { name: 'S3SSM',     desc: 'Save of service/mail segment', cat: 'save' },
    57:  { name: 'S3SM',      desc: 'Service/mail - inter-process messaging', cat: 'active' },
    58:  { name: 'S3SDMWD',   desc: 'Save of disk mirroring watchdog', cat: 'save' },
    59:  { name: 'S3IDMWD',   desc: 'Image of disk mirroring watchdog', cat: 'image' },
    60:  { name: 'S3SXMK',    desc: 'Save of XMSG kernel', cat: 'save' },
    61:  { name: 'S3SXROU',   desc: 'Save of XMSG routing', cat: 'save' },
    62:  { name: 'S3XMK',     desc: 'XMSG kernel - cross-machine message passing', cat: 'active' },
    63:  { name: 'S3XROU',    desc: 'XMSG routing code', cat: 'active' },
    64:  { name: 'S3SDNAM',   desc: 'Save of device-name table', cat: 'save' },
    65:  { name: null,         desc: 'Device-name table (NPL symbol SDNAM, not actual SEGFIL name)', cat: 'active' },
    66:  { name: 'S3SXMFI',   desc: 'Save of XMSG watchdog', cat: 'save' },
    67:  { name: 'S3XMFI',    desc: 'XMSG watchdog - monitors XMSG health', cat: 'active' },
    68:  { name: 'S3SNKSE',   desc: 'Save of NUCLEUS server', cat: 'save' },
    69:  { name: 'S3INKSE',   desc: 'Image of NUCLEUS server', cat: 'image' },
    70:  { name: 'S3SNKNA',   desc: 'Save of NUCLEUS name server', cat: 'save' },
    71:  { name: 'S3INKNA',   desc: 'Image of NUCLEUS name server', cat: 'image' },
    72:  { name: 'S3SU110',   desc: 'Save of ND-110 Microprogram', cat: 'save' },
    73:  { name: 'S3IU110',   desc: 'Image of ND-110 Microprogram', cat: 'image' },
    74:  { name: 'S3SU120',   desc: 'Save of ND-120 Microprogram', cat: 'save' },
    75:  { name: 'S3IU120',   desc: 'Image of ND-120 Microprogram', cat: 'image' },
    76:  { name: 'S3SERWC',   desc: 'Save of ERS Watchdog code', cat: 'save' },
    77:  { name: 'S3IERWC',   desc: 'Image of ERS Watchdog code', cat: 'image' },
    78:  { name: 'S3SERWD',   desc: 'Save of ERS Watchdog data', cat: 'save' },
    79:  { name: 'S3IERWD',   desc: 'Image of ERS Watchdog data', cat: 'image' },
    80:  { name: 'S3SPPRMA',  desc: 'Save of Processor Manager server', cat: 'save' },
    81:  { name: 'S3IPRMA',   desc: 'Image of Processor Manager server', cat: 'image' },
    82:  { name: 'S3SPWRS',   desc: 'Save of PFTCON server', cat: 'save' },
    83:  { name: 'S3IPWRS',   desc: 'Image of PFTCON server', cat: 'image' },
    84:  { name: 'S3SBOPC',   desc: 'Save of BOPCOM Server', cat: 'save' },
    85:  { name: 'S3IBOPC',   desc: 'Image of BOPCOM Server', cat: 'image' },
    86:  { name: 'S3SMTSE',   desc: 'Save of Magnetic Tape server', cat: 'save' },
    87:  { name: 'S3IMTSE',   desc: 'Image of Magnetic Tape server', cat: 'image' },
    88:  { name: 'S3SHDM',    desc: 'Save of HDLC-DMAC', cat: 'save' },
    89:  { name: 'S3IHDM',    desc: 'Image of HDLC-DMAC', cat: 'image' },
    90:  { name: 'S3SFAC',    desc: 'Save of remote file access', cat: 'save' },
    91:  { name: 'S3IFAC',    desc: 'Image of remote file access', cat: 'image' },
    92:  { name: 'S3SNKDAT',  desc: 'Save of NUCLEUS data', cat: 'save' },
    93:  { name: 'S3INKDAT',  desc: 'Image of NUCLEUS data', cat: 'image' }
    // Spooler segments (256+/400+ octal) removed - no verified SEGFIL names available
  };

  var CATEGORY_LABELS = {
    kernel:   'Kernel',
    active:   'Active',
    save:     'Save copy',
    image:    'Image copy',
    data:     'Data',
    nd500:    'ND-500',
    system:   'System',
    reserved: 'Reserved'
  };

  // Resolve segment number to name, or empty string
  function resolveSegmentName(segNum) {
    var entry = SYSTEM_SEGMENTS[segNum];
    return entry ? entry.name : '';
  }

  // Resolve segment number to description, or null
  function resolveSegmentDescription(segNum) {
    var entry = SYSTEM_SEGMENTS[segNum];
    return entry ? entry.desc : null;
  }

  // Resolve segment number to category label, or null
  function resolveSegmentCategory(segNum) {
    var entry = SYSTEM_SEGMENTS[segNum];
    if (!entry) return null;
    return CATEGORY_LABELS[entry.cat] || entry.cat;
  }

  window.resolveSegmentName = resolveSegmentName;
  window.resolveSegmentDescription = resolveSegmentDescription;
  window.resolveSegmentCategory = resolveSegmentCategory;
})();
