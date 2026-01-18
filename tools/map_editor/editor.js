/**
 * R-Type J.A.M.E.S. Map Editor
 * 
 * A web-based tool for creating .level.json files by combining
 * World Generation Frames (WGFs).
 */

// ============================================================================
// State Management
// ============================================================================

const state = {
    wgfs: [],           // All loaded WGF definitions
    timeline: [],       // Current level timeline (array of WGF UUIDs)
    selectedFrame: null, // Currently selected frame in timeline
    allTags: new Set(), // All unique tags from WGFs
};

// ============================================================================
// DOM Elements
// ============================================================================

const elements = {
    wgfList: document.getElementById('wgf-list'),
    wgfFolderInput: document.getElementById('wgf-folder-input'),
    searchInput: document.getElementById('search-input'),
    difficultyFilter: document.getElementById('difficulty-filter'),
    tagFilter: document.getElementById('tag-filter'),
    levelTimeline: document.getElementById('level-timeline'),
    levelName: document.getElementById('level-name'),
    levelAuthor: document.getElementById('level-author'),
    levelDescription: document.getElementById('level-description'),
    levelDifficulty: document.getElementById('level-difficulty'),
    levelEndless: document.getElementById('level-endless'),
    frameCount: document.getElementById('frame-count'),
    totalWidth: document.getElementById('total-width'),
    frameDetailsContent: document.getElementById('frame-details-content'),
    jsonPreview: document.getElementById('json-preview'),
    exportBtn: document.getElementById('export-btn'),
    copyBtn: document.getElementById('copy-btn'),
    clearTimelineBtn: document.getElementById('clear-timeline'),
    importLevel: document.getElementById('import-level'),
};

// ============================================================================
// WGF Loading
// ============================================================================

/**
 * Load WGF files from a folder selection.
 */
elements.wgfFolderInput.addEventListener('change', async (e) => {
    const files = Array.from(e.target.files);
    const wgfFiles = files.filter(f => f.name.endsWith('.wgf.json'));
    
    if (wgfFiles.length === 0) {
        alert('No .wgf.json files found in the selected folder.');
        return;
    }

    state.wgfs = [];
    state.allTags.clear();

    for (const file of wgfFiles) {
        try {
            const text = await file.text();
            const wgf = JSON.parse(text);
            wgf._filename = file.name;
            state.wgfs.push(wgf);

            // Collect tags
            if (wgf.tags) {
                wgf.tags.forEach(tag => state.allTags.add(tag));
            }
        } catch (err) {
            console.error(`Failed to parse ${file.name}:`, err);
        }
    }

    // Sort by difficulty then name
    state.wgfs.sort((a, b) => {
        const diffA = a.difficulty || 0;
        const diffB = b.difficulty || 0;
        if (diffA !== diffB) return diffA - diffB;
        return (a.name || '').localeCompare(b.name || '');
    });

    updateTagFilter();
    renderWGFList();
    console.log(`Loaded ${state.wgfs.length} WGF files`);
});

/**
 * Update the tag filter dropdown with all unique tags.
 */
function updateTagFilter() {
    elements.tagFilter.innerHTML = '<option value="all">All Tags</option>';
    const sortedTags = Array.from(state.allTags).sort();
    sortedTags.forEach(tag => {
        const option = document.createElement('option');
        option.value = tag;
        option.textContent = tag;
        elements.tagFilter.appendChild(option);
    });
}

// ============================================================================
// WGF List Rendering
// ============================================================================

/**
 * Render the WGF library list with current filters applied.
 */
function renderWGFList() {
    const searchTerm = elements.searchInput.value.toLowerCase();
    const difficultyFilter = elements.difficultyFilter.value;
    const tagFilter = elements.tagFilter.value;

    const filtered = state.wgfs.filter(wgf => {
        // Search filter
        const nameMatch = (wgf.name || '').toLowerCase().includes(searchTerm);
        const descMatch = (wgf.description || '').toLowerCase().includes(searchTerm);
        if (!nameMatch && !descMatch) return false;

        // Difficulty filter
        const diff = wgf.difficulty || 0;
        if (difficultyFilter === 'easy' && diff > 3) return false;
        if (difficultyFilter === 'medium' && (diff < 3 || diff > 6)) return false;
        if (difficultyFilter === 'hard' && diff < 6) return false;

        // Tag filter
        if (tagFilter !== 'all') {
            if (!wgf.tags || !wgf.tags.includes(tagFilter)) return false;
        }

        return true;
    });

    if (filtered.length === 0) {
        elements.wgfList.innerHTML = '<p class="loading">No frames match your filters</p>';
        return;
    }

    elements.wgfList.innerHTML = filtered.map(wgf => createWGFCard(wgf)).join('');

    // Add event listeners
    elements.wgfList.querySelectorAll('.wgf-card').forEach(card => {
        const uuid = card.dataset.uuid;

        // Drag start
        card.addEventListener('dragstart', (e) => {
            e.dataTransfer.setData('text/plain', uuid);
            card.classList.add('dragging');
        });

        card.addEventListener('dragend', () => {
            card.classList.remove('dragging');
        });

        // Click to add
        card.addEventListener('click', () => {
            addToTimeline(uuid);
        });

        // Show details on hover
        card.addEventListener('mouseenter', () => {
            showFrameDetails(uuid);
        });
    });
}

/**
 * Create HTML for a WGF card.
 */
function createWGFCard(wgf) {
    const diff = wgf.difficulty || 0;
    const diffClass = diff <= 3 ? 'easy' : diff <= 6 ? 'medium' : 'hard';
    const tags = (wgf.tags || []).slice(0, 3).map(t => `<span class="tag">${t}</span>`).join('');
    const width = wgf.width || 800;
    const enemyCount = (wgf.enemies || []).length;
    const obstacleCount = (wgf.obstacles || []).length;

    return `
        <div class="wgf-card" draggable="true" data-uuid="${wgf.uuid}">
            <div class="name">${escapeHtml(wgf.name || 'Unnamed')}</div>
            <div class="meta">
                <span class="difficulty ${diffClass}">${diff.toFixed(1)}</span>
                <span>W:${width}</span>
                <span>👾${enemyCount}</span>
                <span>🧱${obstacleCount}</span>
                ${tags}
            </div>
        </div>
    `;
}

// Filter event listeners
elements.searchInput.addEventListener('input', renderWGFList);
elements.difficultyFilter.addEventListener('change', renderWGFList);
elements.tagFilter.addEventListener('change', renderWGFList);

// ============================================================================
// Timeline Management
// ============================================================================

/**
 * Add a WGF to the timeline.
 */
function addToTimeline(uuid, index = -1) {
    if (index === -1) {
        state.timeline.push(uuid);
    } else {
        state.timeline.splice(index, 0, uuid);
    }
    renderTimeline();
    updateJsonPreview();
}

/**
 * Remove a frame from the timeline.
 */
function removeFromTimeline(index) {
    state.timeline.splice(index, 1);
    renderTimeline();
    updateJsonPreview();
}

/**
 * Clear the entire timeline.
 */
function clearTimeline() {
    if (state.timeline.length === 0) return;
    if (!confirm('Are you sure you want to clear all frames?')) return;
    state.timeline = [];
    state.selectedFrame = null;
    renderTimeline();
    updateJsonPreview();
}

/**
 * Render the timeline.
 */
function renderTimeline() {
    if (state.timeline.length === 0) {
        elements.levelTimeline.innerHTML = `
            <div class="timeline-placeholder">
                <p>👆 Drag frames from the library to build your level</p>
                <p class="hint">Or click a frame to add it to the end</p>
            </div>
        `;
        elements.frameCount.textContent = '0';
        elements.totalWidth.textContent = '0';
        return;
    }

    let totalWidth = 0;
    const html = state.timeline.map((uuid, index) => {
        const wgf = state.wgfs.find(w => w.uuid === uuid);
        const name = wgf ? wgf.name : 'Unknown';
        const width = wgf ? (wgf.width || 800) : 800;
        totalWidth += width;
        const selected = state.selectedFrame === index ? 'selected' : '';

        return `
            <div class="timeline-frame ${selected}" data-index="${index}" data-uuid="${uuid}">
                <span class="index">${index + 1}</span>
                <span class="frame-name" title="${escapeHtml(name)}">${escapeHtml(name)}</span>
                <button class="remove-btn" title="Remove frame">×</button>
            </div>
        `;
    }).join('');

    elements.levelTimeline.innerHTML = html;
    elements.frameCount.textContent = state.timeline.length;
    elements.totalWidth.textContent = totalWidth.toLocaleString();

    // Add event listeners
    elements.levelTimeline.querySelectorAll('.timeline-frame').forEach(frame => {
        const index = parseInt(frame.dataset.index);
        const uuid = frame.dataset.uuid;

        // Select on click
        frame.addEventListener('click', (e) => {
            if (!e.target.classList.contains('remove-btn')) {
                state.selectedFrame = index;
                renderTimeline();
                showFrameDetails(uuid);
            }
        });

        // Remove button
        frame.querySelector('.remove-btn').addEventListener('click', (e) => {
            e.stopPropagation();
            removeFromTimeline(index);
        });

        // Drag for reordering
        frame.draggable = true;
        frame.addEventListener('dragstart', (e) => {
            e.dataTransfer.setData('text/plain', `timeline:${index}`);
            frame.classList.add('dragging');
        });
        frame.addEventListener('dragend', () => {
            frame.classList.remove('dragging');
        });
    });
}

// Timeline drag and drop
elements.levelTimeline.addEventListener('dragover', (e) => {
    e.preventDefault();
    elements.levelTimeline.classList.add('drag-over');
});

elements.levelTimeline.addEventListener('dragleave', () => {
    elements.levelTimeline.classList.remove('drag-over');
});

elements.levelTimeline.addEventListener('drop', (e) => {
    e.preventDefault();
    elements.levelTimeline.classList.remove('drag-over');

    const data = e.dataTransfer.getData('text/plain');

    if (data.startsWith('timeline:')) {
        // Reordering within timeline
        const fromIndex = parseInt(data.split(':')[1]);
        const frames = elements.levelTimeline.querySelectorAll('.timeline-frame');
        let toIndex = frames.length; // Default to end

        // Find drop position
        for (const frame of frames) {
            const rect = frame.getBoundingClientRect();
            if (e.clientX < rect.left + rect.width / 2) {
                toIndex = parseInt(frame.dataset.index);
                break;
            }
        }

        // Move frame
        if (fromIndex !== toIndex) {
            const uuid = state.timeline[fromIndex];
            state.timeline.splice(fromIndex, 1);
            if (toIndex > fromIndex) toIndex--;
            state.timeline.splice(toIndex, 0, uuid);
            renderTimeline();
            updateJsonPreview();
        }
    } else {
        // Adding new frame from library
        addToTimeline(data);
    }
});

elements.clearTimelineBtn.addEventListener('click', clearTimeline);

// ============================================================================
// Frame Details
// ============================================================================

/**
 * Show details for a WGF frame.
 */
function showFrameDetails(uuid) {
    const wgf = state.wgfs.find(w => w.uuid === uuid);
    if (!wgf) {
        elements.frameDetailsContent.innerHTML = '<p class="placeholder">Frame not found</p>';
        return;
    }

    const enemies = wgf.enemies || [];
    const obstacles = wgf.obstacles || [];
    const tags = wgf.tags || [];

    let enemyList = enemies.length > 0 
        ? enemies.map(e => `<div class="detail-list-item">👾 ${e.tag} at (${e.position?.x || 0}, ${e.position?.y || 0})</div>`).join('')
        : '<div class="detail-list-item">No enemies</div>';

    let obstacleList = obstacles.length > 0
        ? obstacles.map(o => `<div class="detail-list-item">🧱 ${o.type || 'static'} (${o.size?.width || 0}x${o.size?.height || 0})</div>`).join('')
        : '<div class="detail-list-item">No obstacles</div>';

    elements.frameDetailsContent.innerHTML = `
        <div class="detail-row">
            <span class="label">Name</span>
            <span class="value">${escapeHtml(wgf.name || 'Unnamed')}</span>
        </div>
        <div class="detail-row">
            <span class="label">UUID</span>
            <span class="value" style="font-size: 0.7rem;">${wgf.uuid}</span>
        </div>
        <div class="detail-row">
            <span class="label">Difficulty</span>
            <span class="value">${(wgf.difficulty || 0).toFixed(1)}</span>
        </div>
        <div class="detail-row">
            <span class="label">Width</span>
            <span class="value">${wgf.width || 800} units</span>
        </div>
        <div class="detail-row">
            <span class="label">Tags</span>
            <span class="value">${tags.join(', ') || 'None'}</span>
        </div>
        ${wgf.description ? `<div class="detail-row"><span class="label">Description</span></div><p style="font-size: 0.8rem; color: var(--text-secondary); margin-top: 0.25rem;">${escapeHtml(wgf.description)}</p>` : ''}
        
        <div class="detail-section">
            <h4>Enemies (${enemies.length})</h4>
            <div class="detail-list">${enemyList}</div>
        </div>
        
        <div class="detail-section">
            <h4>Obstacles (${obstacles.length})</h4>
            <div class="detail-list">${obstacleList}</div>
        </div>
    `;
}

// ============================================================================
// Export / Import
// ============================================================================

/**
 * Generate the level JSON object.
 */
function generateLevelJSON() {
    const uuid = generateUUID();
    return {
        uuid: uuid,
        name: elements.levelName.value || 'Untitled Level',
        author: elements.levelAuthor.value || 'Anonymous',
        description: elements.levelDescription.value || '',
        target_difficulty: parseFloat(elements.levelDifficulty.value) || 5.0,
        is_endless: elements.levelEndless.checked,
        frames: [...state.timeline]
    };
}

/**
 * Update the JSON preview.
 */
function updateJsonPreview() {
    const json = generateLevelJSON();
    elements.jsonPreview.querySelector('code').textContent = JSON.stringify(json, null, 2);
}

// Update preview when level info changes
[elements.levelName, elements.levelAuthor, elements.levelDescription, 
 elements.levelDifficulty, elements.levelEndless].forEach(el => {
    el.addEventListener('input', updateJsonPreview);
    el.addEventListener('change', updateJsonPreview);
});

/**
 * Export level as .level.json file.
 */
elements.exportBtn.addEventListener('click', () => {
    if (state.timeline.length === 0) {
        alert('Add some frames to your level first!');
        return;
    }

    const json = generateLevelJSON();
    const safeName = (json.name || 'level').toLowerCase().replace(/[^a-z0-9]+/g, '_');
    const filename = `${safeName}.level.json`;

    // Try to POST to local save server first
    (async () => {
        try {
            const resp = await fetch('http://localhost:4321/save-level', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ filename: filename, content: json })
            });

            if (resp.ok) {
                const data = await resp.json();
                console.log('Saved level to server:', data.path);
                alert('Level saved to server assets folder successfully.');
                return;
            } else {
                console.warn('Save server responded with', resp.status);
            }
        } catch (err) {
            console.warn('Save server not available:', err.message);
        }

        // Fallback: download file locally
        const blob = new Blob([JSON.stringify(json, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);

        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        a.click();

        URL.revokeObjectURL(url);
        console.log(`Exported level: ${filename} (download)`);
        alert('Save server not available; downloaded level locally instead.');
    })();
});

/**
 * Copy JSON to clipboard.
 */
elements.copyBtn.addEventListener('click', async () => {
    const json = generateLevelJSON();
    try {
        await navigator.clipboard.writeText(JSON.stringify(json, null, 2));
        elements.copyBtn.textContent = '✓ Copied!';
        setTimeout(() => {
            elements.copyBtn.textContent = '📋 Copy JSON to Clipboard';
        }, 2000);
    } catch (err) {
        alert('Failed to copy to clipboard');
    }
});

/**
 * Import a level file.
 */
elements.importLevel.addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;

    try {
        const text = await file.text();
        const level = JSON.parse(text);

        // Validate
        if (!level.frames || !Array.isArray(level.frames)) {
            throw new Error('Invalid level file: missing frames array');
        }

        // Load level info
        elements.levelName.value = level.name || 'Imported Level';
        elements.levelAuthor.value = level.author || '';
        elements.levelDescription.value = level.description || '';
        elements.levelDifficulty.value = level.target_difficulty || 5.0;
        elements.levelEndless.checked = level.is_endless || false;

        // Load timeline (only frames that exist in our library)
        state.timeline = level.frames.filter(uuid => 
            state.wgfs.some(w => w.uuid === uuid)
        );

        const skipped = level.frames.length - state.timeline.length;
        if (skipped > 0) {
            alert(`Warning: ${skipped} frame(s) were skipped because they are not in your WGF library.`);
        }

        renderTimeline();
        updateJsonPreview();
        console.log(`Imported level: ${level.name}`);
    } catch (err) {
        alert(`Failed to import level: ${err.message}`);
    }
});

// ============================================================================
// Utilities
// ============================================================================

/**
 * Generate a UUIDv4.
 */
function generateUUID() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        const r = Math.random() * 16 | 0;
        const v = c === 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}

/**
 * Escape HTML to prevent XSS.
 */
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// ============================================================================
// Initialization
// ============================================================================

// Initial render
renderWGFList();
updateJsonPreview();

console.log('R-Type J.A.M.E.S. Map Editor loaded');
console.log('To get started:');
console.log('1. Click "Choose Folder" and select server/assets/worldgen/core/');
console.log('2. Drag frames from the library to the timeline');
console.log('3. Configure level settings and export!');
