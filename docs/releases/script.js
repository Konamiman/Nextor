// ============================================================================
// Common functionality shared between single-page and multi-page modes
// ============================================================================

// Format bytes to human-readable size
function formatBytes(bytes) {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
}

let currentCategoryId = null;
const collapsedReleases = new Set();

// Cache frequently accessed elements
let cachedReleasesList = null;
let cachedNavItems = null;
let cachedSidebarFooter = null;
let loadingOverlay = null;

function getReleasesList() {
  if (!cachedReleasesList) {
    cachedReleasesList = document.getElementById('releasesList');
  }
  return cachedReleasesList;
}

function getNavItems() {
  if (!cachedNavItems) {
    cachedNavItems = document.querySelectorAll('.nav-item[data-category-id]');
  }
  return cachedNavItems;
}

function getSidebarFooter() {
  if (!cachedSidebarFooter) {
    cachedSidebarFooter = document.querySelector('.sidebar-footer[data-listed-all]');
  }
  return cachedSidebarFooter;
}

function getLoadingOverlay() {
  if (!loadingOverlay) {
    loadingOverlay = document.getElementById('loadingOverlay');
  }
  return loadingOverlay;
}

function showLoading() {
  const overlay = getLoadingOverlay();
  if (overlay) {
    overlay.classList.add('show');
    // Force a reflow to ensure spinner is painted before heavy work
    overlay.offsetHeight;
  }
}

function hideLoading() {
  const overlay = getLoadingOverlay();
  if (overlay) {
    overlay.classList.remove('show');
  }
}

// Process items in chunks to keep UI responsive
function processInChunks(items, processItem, chunkSize = 50) {
  return new Promise(resolve => {
    let index = 0;

    function processChunk() {
      const end = Math.min(index + chunkSize, items.length);
      while (index < end) {
        processItem(items[index]);
        index++;
      }

      if (index < items.length) {
        requestAnimationFrame(processChunk);
      } else {
        resolve();
      }
    }

    processChunk();
  });
}

function toggleNavCategory(categoryId) {
  const navItem = document.querySelector(`.nav-item[data-category-id="${categoryId}"]`);
  if (navItem) {
    navItem.classList.toggle('expanded');
  }
}

function updateSidebarSelection(categoryId, wasAlreadySelected) {
  const navItem = document.querySelector(`.nav-item[data-category-id="${categoryId}"]`);

  // Update sidebar selection
  const navItems = getNavItems();
  for (let i = 0; i < navItems.length; i++) {
    navItems[i].classList.remove('selected');
  }

  if (navItem) {
    navItem.classList.add('selected');

    if (navItem.classList.contains('has-children')) {
      if (wasAlreadySelected) {
        navItem.classList.toggle('expanded');
      } else {
        navItem.classList.add('expanded');
      }
    }

    let parent = navItem.parentElement.closest('.nav-item');
    while (parent) {
      parent.classList.add('expanded');
      parent = parent.parentElement.closest('.nav-item');
    }
  }
}

function toggleLatestOnly(checked) {
  showLoading();
  setTimeout(() => {
    document.body.classList.toggle('latest-only', checked);
    updateEmptyState();
    updateSidebarCounts();
    updateTruncationBanner();
    updateListedCount();
    updateUrlParams();
    hideLoading();
  }, 10);
}

function togglePrereleases(checked) {
  showLoading();
  setTimeout(() => {
    document.body.classList.toggle('hide-prereleases', !checked);
    updateEmptyState();
    updateSidebarCounts();
    updateTruncationBanner();
    updateListedCount();
    updateUrlParams();
    hideLoading();
  }, 10);
}

function toggleAssets(checked) {
  showLoading();
  setTimeout(() => {
    document.body.classList.toggle('hide-assets', !checked);
    updateUrlParams();
    hideLoading();
  }, 10);
}

function toggleReleaseCard(card) {
  const releaseId = card.dataset.releaseId;
  card.classList.toggle('collapsed');
  if (card.classList.contains('collapsed')) {
    collapsedReleases.add(releaseId);
  } else {
    collapsedReleases.delete(releaseId);
  }
}

function restoreCollapsedStates() {
  if (collapsedReleases.size === 0) return;

  const cards = getReleasesList().getElementsByClassName('release-card');
  for (let i = 0; i < cards.length; i++) {
    const card = cards[i];
    if (collapsedReleases.has(card.dataset.releaseId)) {
      card.classList.add('collapsed');
    }
  }
}

function expandAllReleases() {
  showLoading();
  setTimeout(() => {
    const cards = getReleasesList().getElementsByClassName('release-card');
    if (cards.length === 0) {
      hideLoading();
      return;
    }
    doExpandAllReleases(cards);
  }, 10);
}

async function doExpandAllReleases(cards) {
  const cardArray = Array.from(cards);

  await processInChunks(cardArray, card => {
    card.classList.remove('collapsed');
    collapsedReleases.delete(card.dataset.releaseId);
  }, 100);

  hideLoading();
}

function collapseAllReleases() {
  showLoading();
  setTimeout(() => {
    const cards = getReleasesList().getElementsByClassName('release-card');
    if (cards.length === 0) {
      hideLoading();
      return;
    }
    doCollapseAllReleases(cards);
  }, 10);
}

async function doCollapseAllReleases(cards) {
  const cardArray = Array.from(cards);

  await processInChunks(cardArray, card => {
    card.classList.add('collapsed');
    collapsedReleases.add(card.dataset.releaseId);
  }, 100);

  hideLoading();
}

function updateEmptyState() {
  const releasesList = getReleasesList();
  const cards = releasesList.getElementsByClassName('release-card');
  const noMatchesEl = releasesList.querySelector('.no-matches');
  const descriptionEl = document.getElementById('categoryDescription');

  // No release cards at all (e.g., filtered out by include/exclude)
  if (cards.length === 0) {
    if (noMatchesEl) noMatchesEl.style.display = 'block';
    if (descriptionEl) {
      descriptionEl.style.display = 'none';
    }
    return;
  }

  const isLatestOnly = document.body.classList.contains('latest-only');
  const isHidePrereleases = document.body.classList.contains('hide-prereleases');

  if (!isLatestOnly && !isHidePrereleases) {
    if (noMatchesEl) noMatchesEl.style.display = 'none';
    // Restore description if it has content
    if (descriptionEl && descriptionEl.innerHTML.trim()) {
      descriptionEl.style.display = 'block';
    }
    return;
  }

  let hasVisible = false;
  for (let i = 0; i < cards.length; i++) {
    const card = cards[i];
    const isHiddenByLatest = isLatestOnly && card.dataset.isLatest !== 'true';
    const isHiddenByPrerelease = isHidePrereleases && card.dataset.isPrerelease === 'true';
    if (!isHiddenByLatest && !isHiddenByPrerelease) {
      hasVisible = true;
      break;
    }
  }

  if (noMatchesEl) {
    noMatchesEl.style.display = hasVisible ? 'none' : 'block';
  }

  // Show/hide description based on whether releases are visible
  if (descriptionEl && descriptionEl.innerHTML.trim()) {
    descriptionEl.style.display = hasVisible ? 'block' : 'none';
  }
}

function updateSidebarCounts() {
  const latestOnly = document.body.classList.contains('latest-only');
  const hidePrereleases = document.body.classList.contains('hide-prereleases');

  let countAttr;
  if (latestOnly && hidePrereleases) {
    countAttr = 'countLatestnopre';
  } else if (latestOnly) {
    countAttr = 'countLatest';
  } else if (hidePrereleases) {
    countAttr = 'countNopre';
  } else {
    countAttr = 'countAll';
  }

  const navItems = getNavItems();
  let allHidden = true;
  for (let i = 0; i < navItems.length; i++) {
    const navItem = navItems[i];
    const countEl = navItem.querySelector(':scope > .nav-link > .nav-count[data-count-all]');
    if (!countEl) continue;

    const count = countEl.dataset[countAttr];
    countEl.textContent = count;

    if (count === '0') {
      navItem.classList.add('hidden-empty');
    } else {
      navItem.classList.remove('hidden-empty');
      allHidden = false;
    }
  }

  // Show/hide "No matching categories" message
  const noCategoriesEl = document.querySelector('.no-categories');
  if (noCategoriesEl) {
    if (allHidden && navItems.length > 0) {
      noCategoriesEl.classList.add('visible');
    } else {
      noCategoriesEl.classList.remove('visible');
    }
  }
}

function updateTruncationBanner() {
  const banner = document.querySelector('.releases-truncated');
  if (!banner) return;

  const latestOnly = document.body.classList.contains('latest-only');
  const hidePrereleases = document.body.classList.contains('hide-prereleases');

  let displayed, matching;
  if (latestOnly && hidePrereleases) {
    displayed = banner.dataset.displayedLatestnopre;
    matching = banner.dataset.matchingLatestnopre;
  } else if (latestOnly) {
    displayed = banner.dataset.displayedLatest;
    matching = banner.dataset.matchingLatest;
  } else if (hidePrereleases) {
    displayed = banner.dataset.displayedNopre;
    matching = banner.dataset.matchingNopre;
  } else {
    displayed = banner.dataset.displayedAll;
    matching = banner.dataset.matchingAll;
  }

  const textEl = banner.querySelector('.truncated-text');
  if (textEl) {
    textEl.textContent = `Displaying ${displayed} of ${matching} matching releases`;
  }

  banner.style.display = (displayed === matching) ? 'none' : 'flex';

  const popup = banner.querySelector('.truncated-info-popup');
  if (popup) {
    popup.classList.remove('show');
  }
}

function updateListedCount() {
  const footer = getSidebarFooter();
  if (!footer) return;

  const latestOnly = document.body.classList.contains('latest-only');
  const hidePrereleases = document.body.classList.contains('hide-prereleases');

  let count;
  if (latestOnly && hidePrereleases) {
    count = footer.dataset.listedLatestnopre;
  } else if (latestOnly) {
    count = footer.dataset.listedLatest;
  } else if (hidePrereleases) {
    count = footer.dataset.listedNopre;
  } else {
    count = footer.dataset.listedAll;
  }

  const existing = footer.dataset.existing;
  const allListed = count === existing;

  const countEl = footer.querySelector('.listed-count');
  if (countEl) {
    countEl.textContent = allListed
      ? `${count} releases listed`
      : `${count} of ${existing} releases listed`;
  }

  const infoIcon = footer.querySelector('.info-icon');
  if (infoIcon) {
    infoIcon.title = allListed
      ? `${existing} releases exist in the repository, all are listed on this site`
      : `${existing} releases exist in the repository, ${count} listed on this site`;
  }
}

function toggleTruncationInfo(btn) {
  const banner = btn.closest('.releases-truncated');
  const wrapper = btn.closest('.truncated-info-wrapper');
  const popup = wrapper.querySelector('.truncated-info-popup');
  const popupContent = popup.querySelector('.popup-content');

  if (popup.classList.contains('show')) {
    popup.classList.remove('show');
    return;
  }

  const siteListed = banner.dataset.siteListed;
  const siteExisting = banner.dataset.siteExisting;
  const categoryName = banner.dataset.categoryName;
  const displayedAll = banner.dataset.displayedAll;
  const matchingAll = banner.dataset.matchingAll;

  const latestOnly = document.body.classList.contains('latest-only');
  const hidePrereleases = document.body.classList.contains('hide-prereleases');
  const hasFilters = latestOnly || hidePrereleases;

  let displayedFiltered, matchingFiltered;
  if (latestOnly && hidePrereleases) {
    displayedFiltered = banner.dataset.displayedLatestnopre;
    matchingFiltered = banner.dataset.matchingLatestnopre;
  } else if (latestOnly) {
    displayedFiltered = banner.dataset.displayedLatest;
    matchingFiltered = banner.dataset.matchingLatest;
  } else if (hidePrereleases) {
    displayedFiltered = banner.dataset.displayedNopre;
    matchingFiltered = banner.dataset.matchingNopre;
  }

  let html = `
    <div class="popup-section">
      <div class="popup-row">
        <span>Releases in repository:</span>
        <span class="popup-value">${siteExisting}</span>
      </div>
      <div class="popup-row">
        <span>Releases listed on this site:</span>
        <span class="popup-value">${siteListed}</span>
      </div>
    </div>
    <div class="popup-section">
      <div class="popup-label">Releases in "${categoryName}" category:</div>
      <div class="popup-indent">
        <div class="popup-row">
          <span>Matching:</span>
          <span class="popup-value">${matchingAll}</span>
        </div>
        <div class="popup-row">
          <span>Displayed:</span>
          <span class="popup-value">${displayedAll}</span>
        </div>
      </div>
    </div>
  `;

  if (hasFilters && (displayedFiltered !== displayedAll || matchingFiltered !== matchingAll)) {
    html += `
    <div class="popup-section">
      <div class="popup-label">With current filters applied:</div>
      <div class="popup-indent">
        <div class="popup-row">
          <span>Matching:</span>
          <span class="popup-value">${matchingFiltered}</span>
        </div>
        <div class="popup-row">
          <span>Displayed:</span>
          <span class="popup-value">${displayedFiltered}</span>
        </div>
      </div>
    </div>
    `;
  }

  popupContent.innerHTML = html;
  popup.classList.add('show');

  function closeOnClickOutside(e) {
    if (!popup.contains(e.target) && e.target !== btn) {
      popup.classList.remove('show');
      document.removeEventListener('click', closeOnClickOutside);
    }
  }
  setTimeout(() => document.addEventListener('click', closeOnClickOutside), 0);
}

// Format dates using browser locale
function formatDates() {
  const elements = getReleasesList().querySelectorAll('.date-value[data-date]');
  const dateOptions = { year: 'numeric', month: 'short', day: 'numeric' };

  for (let i = 0; i < elements.length; i++) {
    const el = elements[i];
    const isoDate = el.dataset.date;
    if (isoDate && !el.textContent) {
      const date = new Date(isoDate);
      el.textContent = date.toLocaleDateString(undefined, dateOptions);
    }
  }

  // Also format dates in latest assets table
  formatLatestAssetsDates();
}

// Theme toggle functionality
function initThemeToggle() {
  const toggle = document.getElementById('themeToggle');
  if (!toggle) return;

  const iconSun = toggle.querySelector('.icon-sun');
  const iconMoon = toggle.querySelector('.icon-moon');

  function getEffectiveTheme() {
    const stored = localStorage.getItem('theme');
    if (stored === 'dark' || stored === 'light') {
      return stored;
    }
    return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  }

  function updateIcon() {
    const theme = getEffectiveTheme();
    if (theme === 'dark') {
      iconSun.style.display = 'none';
      iconMoon.style.display = 'block';
    } else {
      iconSun.style.display = 'block';
      iconMoon.style.display = 'none';
    }
  }

  const storedTheme = localStorage.getItem('theme');
  if (storedTheme === 'dark' || storedTheme === 'light') {
    document.documentElement.setAttribute('data-theme', storedTheme);
  }
  updateIcon();

  toggle.addEventListener('click', () => {
    const currentTheme = getEffectiveTheme();
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', newTheme);
    localStorage.setItem('theme', newTheme);
    updateIcon();
  });

  window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', () => {
    if (!localStorage.getItem('theme')) {
      updateIcon();
    }
  });
}

// Initialize theme toggle on DOM ready
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', initThemeToggle);
} else {
  initThemeToggle();
}

// ============================================================================
// Latest Page Display Modes
// ============================================================================

let currentLatestDisplayMode = 'releases';

function setLatestDisplayMode(mode) {
  currentLatestDisplayMode = mode;

  // Update button active states
  const buttons = document.querySelectorAll('.display-mode-btn');
  buttons.forEach(btn => {
    btn.classList.toggle('active', btn.dataset.mode === mode);
  });

  // Toggle views
  const releasesView = document.querySelector('.latest-releases-view');
  const assetsView = document.querySelector('.latest-assets-view');

  if (mode === 'releases') {
    if (releasesView) releasesView.style.display = 'block';
    if (assetsView) assetsView.style.display = 'none';
  } else {
    if (releasesView) releasesView.style.display = 'none';
    if (assetsView) assetsView.style.display = 'block';
    sortLatestAssets(mode);
  }

  // Update URL params
  if (typeof updateUrlParams === 'function') {
    updateUrlParams();
  }
}

function sortLatestAssets(mode) {
  const tbody = document.querySelector('.latest-assets-table tbody');
  if (!tbody) return;

  const rows = Array.from(tbody.querySelectorAll('.latest-asset-row'));
  if (rows.length === 0) return;

  rows.sort((a, b) => {
    switch (mode) {
      case 'assets-date-desc':
        return new Date(b.dataset.date) - new Date(a.dataset.date);
      case 'assets-date-asc':
        return new Date(a.dataset.date) - new Date(b.dataset.date);
      case 'assets-name-asc':
        return a.dataset.name.localeCompare(b.dataset.name);
      case 'assets-name-desc':
        return b.dataset.name.localeCompare(a.dataset.name);
      default:
        return 0;
    }
  });

  // Re-append in sorted order
  rows.forEach(row => tbody.appendChild(row));
}

function formatLatestAssetsDates() {
  const elements = document.querySelectorAll('.latest-assets-table .date-value[data-date]');
  const dateOptions = { year: 'numeric', month: 'short', day: 'numeric' };

  for (let i = 0; i < elements.length; i++) {
    const el = elements[i];
    const isoDate = el.dataset.date;
    if (isoDate && !el.textContent) {
      const date = new Date(isoDate);
      el.textContent = date.toLocaleDateString(undefined, dateOptions);
    }
  }
}


// ============================================================================
// Single-page mode specific functionality
// Requires: script-common.js to be loaded first
// ============================================================================

function updateUrlParams() {
  const params = new URLSearchParams();

  const latestOnly = document.getElementById('showLatestOnly');
  const prereleases = document.getElementById('showPrereleases');
  const assets = document.getElementById('showAssets');

  if (latestOnly && latestOnly.checked) params.set('latest', '1');
  if (prereleases && !prereleases.checked) params.set('prereleases', '0');
  if (assets && !assets.checked) params.set('assets', '0');

  const queryString = params.toString();
  const hash = window.location.hash;
  const newUrl = window.location.pathname + (queryString ? '?' + queryString : '') + hash;
  history.replaceState(null, '', newUrl);
}

function applyUrlParams() {
  const params = new URLSearchParams(window.location.search);

  const latestOnly = document.getElementById('showLatestOnly');
  const prereleases = document.getElementById('showPrereleases');
  const assets = document.getElementById('showAssets');

  if (params.has('latest')) {
    const val = params.get('latest') === '1';
    if (latestOnly) {
      latestOnly.checked = val;
      document.body.classList.toggle('latest-only', val);
    }
  }

  if (params.has('prereleases')) {
    const val = params.get('prereleases') !== '0';
    if (prereleases) {
      prereleases.checked = val;
      document.body.classList.toggle('hide-prereleases', !val);
    }
  }

  if (params.has('assets')) {
    const val = params.get('assets') !== '0';
    if (assets) {
      assets.checked = val;
      document.body.classList.toggle('hide-assets', !val);
    }
  }
}

function selectCategory(categoryId) {
  // Check if this is a large category first
  const html = categoryReleasesHtml[categoryId];
  const isLargeCategory = html && html.length > 50000;

  // Show loading immediately for large categories
  if (isLargeCategory) {
    showLoading();
    setTimeout(() => doSelectCategoryFull(categoryId, html), 10);
  } else {
    doSelectCategoryFull(categoryId, html);
  }
}

function doSelectCategoryFull(categoryId, html) {
  const wasAlreadySelected = currentCategoryId === categoryId;
  updateSidebarSelection(categoryId, wasAlreadySelected);
  currentCategoryId = categoryId;
  document.body.classList.toggle('latest-page-active', categoryId === 'latest-page');
  doSelectCategory(categoryId, html);
}

function doSelectCategory(categoryId, html) {
  // Update title
  const category = categoryData[categoryId];
  if (category) {
    document.getElementById('categoryTitle').textContent = category.name;
  }

  // Update description
  const descriptionEl = document.getElementById('categoryDescription');
  const descriptionHtml = categoryDescriptionsHtml[categoryId];
  if (descriptionHtml) {
    descriptionEl.innerHTML = descriptionHtml;
    descriptionEl.style.display = 'block';
  } else {
    descriptionEl.innerHTML = '';
    descriptionEl.style.display = 'none';
  }

  // Update releases list
  const releasesList = getReleasesList();

  if (html) {
    releasesList.innerHTML = html;
    restoreCollapsedStates();
    updateEmptyState();
    formatDates();
    hideLoading();
  } else if (category && category.isIndex) {
    // Index page - just show the description, no releases message
    releasesList.innerHTML = '';
  } else if (category && category.children && category.children.length > 0) {
    // Show subcategories navigation when no releases but has children
    const linksHtml = category.children.map(child =>
      `<li><a href="#${child.id}" onclick="event.preventDefault(); selectCategory('${child.id}')">${child.name}</a></li>`
    ).join('');
    releasesList.innerHTML = `
      <div class="subcategories">
        <h3>Browse subcategories</h3>
        <ul>${linksHtml}</ul>
      </div>
    `;
  } else {
    releasesList.innerHTML = `
      <div class="empty-state">
        <svg viewBox="0 0 24 24" width="48" height="48">
          <path fill="currentColor" d="M5.25 3A2.25 2.25 0 003 5.25v13.5A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V5.25A2.25 2.25 0 0018.75 3H5.25zM4.5 5.25a.75.75 0 01.75-.75h13.5a.75.75 0 01.75.75v13.5a.75.75 0 01-.75.75H5.25a.75.75 0 01-.75-.75V5.25z"/>
        </svg>
        <p>No releases in this category</p>
      </div>
    `;
  }

  // Update URL hash (no hash for index page)
  const hash = categoryId === 'index' ? '' : '#' + categoryId;
  const query = window.location.search;
  history.replaceState(null, '', window.location.pathname + query + hash);
}

// Handle browser back/forward for hash changes
window.addEventListener('hashchange', () => {
  const hash = window.location.hash.slice(1);
  if (hash && categoryData[hash] && hash !== currentCategoryId) {
    selectCategory(hash);
  }
});
