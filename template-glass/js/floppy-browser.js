// floppy-browser.js - Floppy browser modal, search, products, mount

let floppyDatabase = [];
let productsDatabase = [];
let filteredFloppies = [];
let currentMountedFloppy = null;
let selectedProductId = null;
let currentProductFilter = '';

// Close modal handlers
document.getElementById('floppy-modal-close').addEventListener('click', function() {
  closeFloppyBrowser();
});

// Search functionality
document.getElementById('floppy-search').addEventListener('input', function(e) {
  filterFloppies();
});

document.getElementById('search-clear').addEventListener('click', function() {
  document.getElementById('floppy-search').value = '';
  filterFloppies();
});

// Products modal functionality
document.getElementById('select-product-btn').addEventListener('click', function() {
  openProductsModal();
});

document.getElementById('clear-product-btn').addEventListener('click', function() {
  clearProductFilter();
});

document.getElementById('products-modal-close').addEventListener('click', function() {
  closeProductsModal();
});

document.getElementById('products-cancel-btn').addEventListener('click', function() {
  closeProductsModal();
});

document.getElementById('products-ok-btn').addEventListener('click', function() {
  selectProductAndClose();
});

document.getElementById('product-filter').addEventListener('input', function() {
  filterProductsList();
});

document.getElementById('products-modal').addEventListener('click', function(e) {
  if (e.target === this) {
    closeProductsModal();
  }
});

var floppyBrowserDefaultUnit = 0;

function openFloppyBrowser(defaultUnit) {
  if (typeof defaultUnit === 'number') floppyBrowserDefaultUnit = defaultUnit;
  var sel = document.getElementById('floppy-drive-select');
  if (sel) sel.value = String(floppyBrowserDefaultUnit);
  document.getElementById('floppy-modal').style.display = 'flex';
  loadFloppyDatabase();
}

function closeFloppyBrowser() {
  document.getElementById('floppy-modal').style.display = 'none';
}

async function loadFloppyDatabase() {
  const floppyLoading = document.getElementById('floppy-loading');
  const floppyList = document.getElementById('floppy-list');

  floppyLoading.style.display = 'flex';
  floppyList.innerHTML = '';

  try {
    console.log('Loading floppy database...');
    const floppyResponse = await fetch('floppies/floppies.json');
    if (!floppyResponse.ok) {
      throw new Error(`Failed to load floppy database: ${floppyResponse.status}`);
    }
    floppyDatabase = await floppyResponse.json();
    console.log(`Loaded ${floppyDatabase.length} floppies`);

    console.log('Loading products database...');
    const productsResponse = await fetch('floppies/products.json');
    if (!productsResponse.ok) {
      throw new Error(`Failed to load products database: ${productsResponse.status}`);
    }
    productsDatabase = await productsResponse.json();
    console.log(`Loaded ${productsDatabase.length} products`);

    document.getElementById('floppy-browser-main').style.display = 'flex';

    filteredFloppies = floppyDatabase.filter(floppy => floppy.Status === 0);
    displayFloppies();

    floppyLoading.style.display = 'none';

  } catch (error) {
    console.error('Error loading floppy database:', error);
    floppyLoading.innerHTML = `
      <div style="color: #e53935; text-align: center;">
        <p><strong>Error loading floppy database</strong></p>
        <p>${error.message}</p>
        <p><small>Please check your internet connection and try again.</small></p>
      </div>
    `;
  }
}

function updateResultsCount() {
  const resultsCount = document.getElementById('floppy-results-count');
  const count = filteredFloppies.length;
  resultsCount.textContent = `${count} floppy${count !== 1 ? 'ies' : ''} found`;
}

function filterFloppies() {
  const searchTerm = document.getElementById('floppy-search').value.toLowerCase();
  const searchClearBtn = document.getElementById('search-clear');

  searchClearBtn.style.display = searchTerm ? 'block' : 'none';

  filteredFloppies = floppyDatabase.filter(floppy => {
    if (floppy.Status !== 0) return false;

    if (selectedProductId) {
      const selectedProduct = productsDatabase.find(p => p.Id === selectedProductId);
      if (!selectedProduct || !floppyMatchesProduct(floppy, selectedProduct)) {
        return false;
      }
    }

    if (!searchTerm) return true;

    const matchesName = floppy.Name && floppy.Name.toLowerCase().includes(searchTerm);
    const matchesDescription = floppy.Description && floppy.Description.toLowerCase().includes(searchTerm);
    const matchesDirectoryContent = floppy.DirectoryContent && floppy.DirectoryContent.toLowerCase().includes(searchTerm);
    const matchesReference = floppy.Reference && floppy.Reference.toLowerCase().includes(searchTerm);
    const matchesFilePath = floppy.FilePath && floppy.FilePath.toLowerCase().includes(searchTerm);

    let matchesProduct = false;
    if (floppy.Product && floppy.Product.Name) {
      matchesProduct = floppy.Product.Name.toLowerCase().includes(searchTerm);
    }

    return matchesName || matchesDescription || matchesDirectoryContent || matchesReference || matchesFilePath || matchesProduct;
  });

  displayFloppies();
  updateResultsCount();
}

function displayFloppies() {
  const floppyList = document.getElementById('floppy-list');
  floppyList.innerHTML = '';

  if (filteredFloppies.length === 0) {
    floppyList.innerHTML = '<div style="text-align: center; color: #666; padding: 40px;">No floppies found matching your search criteria.</div>';
    updateResultsCount();
    return;
  }

  filteredFloppies.forEach(floppy => {
    const floppyItem = createFloppyItem(floppy);
    floppyList.appendChild(floppyItem);
  });

  updateResultsCount();
}

function createFloppyItem(floppy) {
  const item = document.createElement('div');
  item.className = 'ndlib-floppy-item';
  item.onclick = () => selectFloppy(floppy);

  const name = floppy.Name || 'Unnamed Floppy';
  const description = floppy.Description || 'No description available';

  let productName = 'No Product';
  if (floppy.Product && floppy.Product.Name) {
    productName = floppy.Product.Name;
  } else {
    const matchingProduct = productsDatabase.find(product => floppyMatchesProduct(floppy, product));
    if (matchingProduct) {
      productName = matchingProduct.Name;
    }
  }

  item.innerHTML = `
    <div class="ndlib-floppy-name">${escapeHtml(name)}</div>
    <div class="ndlib-floppy-description">${escapeHtml(description)}</div>
    <div class="ndlib-product-badge">${escapeHtml(productName)}</div>
  `;

  return item;
}

function escapeHtml(text) {
  const div = document.createElement('div');
  div.textContent = text;
  return div.innerHTML;
}

function selectFloppy(floppy) {
  document.querySelectorAll('.ndlib-floppy-item').forEach(item => {
    item.classList.remove('active');
  });

  event.currentTarget.classList.add('active');
  displayFloppyDetails(floppy);
}

function displayFloppyDetails(floppy) {
  const detailsContainer = document.getElementById('floppy-details');

  const name = floppy.Name || 'Unnamed Floppy';
  const description = floppy.Description || 'No description available';
  const md5 = floppy.Md5 || 'N/A';
  const directoryContent = floppy.DirectoryContent || 'No directory content available';

  let productName = 'No Product';
  if (floppy.Product && floppy.Product.Name) {
    productName = floppy.Product.Name;
  } else {
    const matchingProduct = productsDatabase.find(product => floppyMatchesProduct(floppy, product));
    if (matchingProduct) {
      productName = matchingProduct.Name;
    }
  }

  detailsContainer.innerHTML = `
    <div class="floppy-details-content">
      <div class="floppy-details-top">
        <div class="floppy-details-info">
          <h4>${escapeHtml(name)}</h4>
          <div class="floppy-details-row">
            <div class="floppy-details-label">Description:</div>
            <div class="floppy-details-value">${escapeHtml(description)}</div>
          </div>
          <div class="floppy-details-row">
            <div class="floppy-details-label">Product:</div>
            <div class="floppy-details-value">${escapeHtml(productName)}</div>
          </div>
          <div class="floppy-details-row">
            <div class="floppy-details-label">MD5:</div>
            <div class="floppy-details-value">${escapeHtml(md5)}</div>
          </div>
        </div>
        <div class="floppy-details-actions">
          <span class="floppy-drive-select-label" id="floppy-device-name">Device name: FLOPPY-DISC-1</span>
          <label class="floppy-drive-select-label">Device unit:</label>
          <select id="floppy-drive-select" class="floppy-drive-select">
            <option value="0">Unit 0</option>
            <option value="1">Unit 1</option>
          </select>
          <button class="floppy-mount-button" data-floppy='${JSON.stringify(floppy)}'>Mount</button>
        </div>
      </div>
      <h5>Directory Content:</h5>
      <div class="floppy-directory-content">${escapeHtml(directoryContent)}</div>
    </div>
  `;

  // Set default unit from mount button context
  var driveSel = document.getElementById('floppy-drive-select');
  if (driveSel) driveSel.value = String(floppyBrowserDefaultUnit);

  // Wire up mount button
  detailsContainer.querySelector('.floppy-mount-button').addEventListener('click', function() {
    mountFloppy();
  });

}

async function mountFloppy() {
  var mountButton = document.querySelector('.floppy-mount-button');
  var driveSelect = document.getElementById('floppy-drive-select');
  var progressContainer = document.getElementById('floppy-download-progress');
  var progressFill = document.getElementById('progress-fill');
  var progressText = document.getElementById('progress-text');

  if (!mountButton) return;

  var unitNum = driveSelect ? parseInt(driveSelect.value) : 0;  // hardware unit (0 or 1)
  var driveNum = unitNum + 1;  // UI drive number (1 or 2, matches element IDs)
  var floppyData;

  try {
    floppyData = JSON.parse(mountButton.getAttribute('data-floppy'));
  } catch (e) {
    console.error('No floppy data on mount button');
    return;
  }

  console.log('Mounting floppy to FLOPPY-DISC-1 Unit ' + unitNum + ':', floppyData.Name);

  try {
    progressContainer.style.display = 'block';
    progressFill.style.width = '0%';
    progressFill.style.backgroundColor = '';
    progressText.textContent = 'Downloading floppy...';

    mountButton.textContent = 'Downloading...';
    mountButton.disabled = true;

    var md5Hash = floppyData.Md5;
    if (!md5Hash) {
      throw new Error('No MD5 hash available for this floppy');
    }

    var imageUrl = 'https://hackercorp.blob.core.windows.net/upload/ndlib/images/' + md5Hash + '.img';
    console.log('Downloading floppy from: ' + imageUrl);

    var response = await fetch(imageUrl);
    if (!response.ok) {
      throw new Error('Failed to download floppy: ' + response.status + ' ' + response.statusText);
    }

    var contentLength = parseInt(response.headers.get('content-length') || '0');
    var reader = response.body.getReader();
    var receivedLength = 0;
    var chunks = [];

    while (true) {
      var result = await reader.read();
      if (result.done) break;

      chunks.push(result.value);
      receivedLength += result.value.length;

      if (contentLength > 0) {
        var progress = (receivedLength / contentLength) * 100;
        progressFill.style.width = progress + '%';
        progressText.textContent = 'Downloading... ' + Math.round(progress) + '%';
      }
    }

    var floppyImageData = new Uint8Array(receivedLength);
    var position = 0;
    for (var i = 0; i < chunks.length; i++) {
      floppyImageData.set(chunks[i], position);
      position += chunks[i].length;
    }

    if (typeof Module !== 'undefined' && Module.FS) {
      // Write to unit-specific filename: FLOPPY0.IMG, FLOPPY1.IMG, etc.
      var floppyFileName = '/FLOPPY' + unitNum + '.IMG';
      Module.FS.writeFile(floppyFileName, floppyImageData);

      try {
        Module.FS.chmod(floppyFileName, 0o666);
      } catch (e) {
        // chmod not critical on MEMFS
      }

      // Tell the C side to close old FILE* and re-open from MEMFS
      if (Module._RemountFloppy) {
        var result = Module._RemountFloppy(unitNum);
        if (result !== 0) {
          throw new Error('Failed to mount floppy on unit ' + unitNum);
        }
      }

      currentMountedFloppy = floppyData;

      progressText.textContent = 'Floppy mounted to FLOPPY-DISC-1 Unit ' + unitNum + '!';
      progressFill.style.width = '100%';
      progressFill.style.backgroundColor = '#4CAF50';

      mountButton.textContent = 'Mounted!';
      mountButton.disabled = true;

      // Update the drive display and toggle mount/eject buttons
      var driveDisplay = document.getElementById('floppy-drive-' + driveNum + '-name');
      if (driveDisplay) {
        driveDisplay.textContent = floppyData.Name || 'Custom Floppy';
      }
      updateFloppyDriveButtons(driveNum);

      console.log('Floppy mounted to FLOPPY-DISC-1 Unit ' + unitNum + ':', floppyData.Name);

      setTimeout(function() {
        closeFloppyBrowser();
      }, 2000);

    } else {
      throw new Error('Virtual filesystem not available');
    }

  } catch (error) {
    console.error('Error mounting floppy:', error);
    progressText.textContent = 'Error: ' + error.message;
    progressFill.style.backgroundColor = '#e53935';
    mountButton.textContent = 'Mount';
    mountButton.disabled = false;
  }
}

// Helper: update mount/eject button visibility for a drive
function updateFloppyDriveButtons(driveNum) {
  var nameEl = document.getElementById('floppy-drive-' + driveNum + '-name');
  var mountBtn = document.getElementById('floppy-drive-' + driveNum + '-mount');
  var ejectBtn = document.getElementById('floppy-drive-' + driveNum + '-eject');
  if (!nameEl || !mountBtn || !ejectBtn) return;
  var mounted = nameEl.textContent !== 'Empty';
  mountBtn.style.display = mounted ? 'none' : '';
  ejectBtn.style.display = mounted ? '' : 'none';
}

// Floppy drive eject handlers
document.getElementById('floppy-drive-1-eject').addEventListener('click', function() {
  if (Module._UnmountFloppy) Module._UnmountFloppy(0);
  currentMountedFloppy = null;
  document.getElementById('floppy-drive-1-name').textContent = 'Empty';
  updateFloppyDriveButtons(1);
});

document.getElementById('floppy-drive-2-eject').addEventListener('click', function() {
  if (Module._UnmountFloppy) Module._UnmountFloppy(1);
  document.getElementById('floppy-drive-2-name').textContent = 'Empty';
  updateFloppyDriveButtons(2);
});

// Floppy drive mount handlers - open Floppy Library with correct default unit
document.getElementById('floppy-drive-1-mount').addEventListener('click', function() {
  openFloppyBrowser(0);
  if (typeof windowManager !== 'undefined') windowManager.focus('floppy-modal');
});

document.getElementById('floppy-drive-2-mount').addEventListener('click', function() {
  openFloppyBrowser(1);
  if (typeof windowManager !== 'undefined') windowManager.focus('floppy-modal');
});

// Products Modal Functions
function openProductsModal() {
  document.getElementById('products-modal').style.display = 'flex';
  loadProductsWithCounts();
  document.getElementById('product-filter').value = '';
  selectedProductId = null;
  updateProductsOkButton();
}

function closeProductsModal() {
  document.getElementById('products-modal').style.display = 'none';
}

function floppyMatchesProduct(floppy, product) {
  const floppyName = (floppy.Name || '').toUpperCase();
  const productId = product.Id.toUpperCase();

  if (floppyName.startsWith(productId)) return true;

  if (productId.startsWith('ND-')) {
    const numberPart = productId.substring(3);
    if (floppyName.startsWith(numberPart)) return true;
  }

  const productIdWithoutND = productId.startsWith('ND-') ? productId.substring(3) : productId;
  if (floppyName.includes(productIdWithoutND)) return true;

  return false;
}

function loadProductsWithCounts() {
  const productsWithCounts = productsDatabase.map(product => {
    const matchCount = floppyDatabase.filter(floppy => {
      return floppyMatchesProduct(floppy, product);
    }).length;
    return { ...product, matchCount };
  }).sort((a, b) => {
    if (b.matchCount !== a.matchCount) return b.matchCount - a.matchCount;
    return a.Id.localeCompare(b.Id);
  });

  currentProductFilter = '';
  displayProductsList(productsWithCounts);
}

function displayProductsList(products) {
  const productsList = document.getElementById('products-list');

  const filteredProducts = products.filter(product => {
    if (!currentProductFilter) return product.matchCount > 0;
    const matchesId = product.Id.toLowerCase().includes(currentProductFilter);
    const matchesName = product.Name.toLowerCase().includes(currentProductFilter);
    return (matchesId || matchesName) && product.matchCount > 0;
  });

  if (filteredProducts.length === 0) {
    productsList.innerHTML = '<li style="padding: 20px; text-align: center; color: #666;">No products found matching your criteria.</li>';
    return;
  }

  productsList.innerHTML = filteredProducts.map(product => `
    <li class="product-item" data-id="${escapeHtml(product.Id)}" onclick="selectProduct('${escapeHtml(product.Id)}')">
      <div class="product-item-content">
        <div class="product-item-main">
          <strong>${escapeHtml(product.Id)}</strong>: ${escapeHtml(product.Name)}
        </div>
        <div class="product-item-count">
          <span class="floppy-count">${product.matchCount}</span>
        </div>
      </div>
    </li>
  `).join('');
}

function filterProductsList() {
  currentProductFilter = document.getElementById('product-filter').value.toLowerCase();

  const productCounts = {};
  floppyDatabase.forEach(floppy => {
    const productId = floppy.ProductId;
    if (productId) {
      productCounts[productId] = (productCounts[productId] || 0) + 1;
    }
  });

  const productsWithCounts = productsDatabase.map(product => ({
    ...product,
    matchCount: productCounts[product.Id] || 0
  })).sort((a, b) => b.matchCount - a.matchCount);

  displayProductsList(productsWithCounts);
}

function selectProduct(productId) {
  document.querySelectorAll('.product-item').forEach(item => {
    item.classList.remove('selected');
  });

  const selectedItem = document.querySelector(`.product-item[data-id="${productId}"]`);
  if (selectedItem) {
    selectedItem.classList.add('selected');
    selectedProductId = productId;
  }

  updateProductsOkButton();
}

function updateProductsOkButton() {
  const okButton = document.getElementById('products-ok-btn');
  okButton.disabled = !selectedProductId;
}

function selectProductAndClose() {
  if (!selectedProductId) return;

  const selectedProduct = productsDatabase.find(p => p.Id === selectedProductId);
  const selectProductBtn = document.getElementById('select-product-btn');
  const clearProductBtn = document.getElementById('clear-product-btn');

  if (selectedProduct) {
    selectProductBtn.textContent = `${selectedProduct.Id}: ${selectedProduct.Name}`;
    selectProductBtn.style.backgroundColor = '#4CAF50';
    clearProductBtn.style.display = 'block';
  }

  filterFloppies();
  closeProductsModal();
}

function clearProductFilter() {
  selectedProductId = null;
  const selectProductBtn = document.getElementById('select-product-btn');
  const clearProductBtn = document.getElementById('clear-product-btn');

  selectProductBtn.textContent = 'Select Product';
  selectProductBtn.style.backgroundColor = '#FF9800';
  clearProductBtn.style.display = 'none';

  filterFloppies();
}

window.selectProduct = selectProduct;
window.clearProductFilter = clearProductFilter;
