// Import the official gitmojis list
const { gitmojis } = require('gitmojis');

// Build a unique list containing both formats for each entry
// Example: [..., "✨", ":sparkles:", "🐛", ":bug:", ...]
const validTypes = gitmojis.flatMap(g => [g.emoji, g.code]);

module.exports = {
    // Custom parser for your format "Emoji [Scope] Subject"
    parserPreset: {
        parserOpts: {
            // Regex:
            // ^(\S+)                  -> Group 1: The Type (Emoji or Code without space)
            // \s+                     -> Mandatory space
            // (?:\[([^\]]+)\]\s+)?    -> Group 2 (Optional): [Scope] + Space
            // (.+)$                   -> Group 3: Subject
            headerPattern: /^(\S+)\s+(?:\[([^\]]+)\]\s+)?(.+)$/,
            headerCorrespondence: ['type', 'scope', 'subject'],
        },
    },

    rules: {
        // The type (emoji) must be present
        'type-empty': [2, 'never'],

        // The type must belong to our merged list (Unicode + Code)
        'type-enum': [2, 'always', validTypes],

        // The subject must not be empty
        'subject-empty': [2, 'never'],

        // Disable forced case to accept uppercase in the subject
        'subject-case': [0],

        // Disable length limit
        'header-max-length': [0],
    },
};
