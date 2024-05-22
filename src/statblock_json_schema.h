#pragma once

const char* jsonSchema = R"(
    {
        "type": "object",
        "properties": {
            "name": {
                "type": "string"
            },
            "level": {
                "type": "number"
            },
            "proficiency": {
                "type": "number"
            },
            "max_hp": {
                "type": "number"
            },
            "armour_class": {
                "type": "number"
            },
            "resources": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "name": "string",
                        "count": "number",
                        "recovery": "string"
                    },
                    "required": [
                        "name",
                        "count"
                    ]
                }
            },
            "actions": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "name": "string",
                        "type": "string",
                        "damage": {
                            "type": "object",
                            "properties": {
                                "acid": "string",
                                "force": "string",
                                "cold": "string",
                                "fire": "string",
                                "lightning": "string",
                                "necrosis": "string",
                                "piercing": "string",
                                "slashing": "string",
                                "bludgeoning": "string",
                                "magic_piercing": "string",
                                "magic_slashing": "string",
                                "magic_bludgeoning": "string",
                                "poison": "string",
                                "radiant": "string",
                                "thunder": "string"
                            }
                        },
                        "crit_damage": {
                            "type": "object",
                            "properties": {
                                "acid": "string",
                                "force": "string",
                                "cold": "string",
                                "fire": "string",
                                "lightning": "string",
                                "necrosis": "string",
                                "piercing": "string",
                                "slashing": "string",
                                "bludgeoning": "string",
                                "magic_piercing": "string",
                                "magic_slashing": "string",
                                "magic_bludgeoning": "string",
                                "poison": "string",
                                "radiant": "string",
                                "thunder": "string"
                            }
                        },
                        "attack": "string",
                        "consume_resources": {
                            "type": "array",
                            "items": {
                                "type": "object",
                                "properties": {
                                    "name": "string",
                                    "count": "number"
                                }
                            }
                        },
                        "consume_on_success": {
                            "type": "array",
                            "items": {
                                "type": "object",
                                "properties": {
                                    "name": "string",
                                    "count": "number"
                                }
                            }
                        },
                        "recover_resources": {
                            "type": "array",
                            "items": {
                                "type": "object",
                                "properties": {
                                    "name": "string",
                                    "count": "number"
                                }
                            }
                        }
                    }
                }
            },
            "stats": {
                "type": "object",
                "properties": {
                    "strength": "number",
                    "dexterity": "number",
                    "constitution": "number",
                    "intelligence": "number",
                    "wisdom": "number",
                    "charisma": "number"
                }
            },
            "savethrows": {
                "type": "object",
                "properties": {
                    "strength": "string",
                    "dexterity": "string",
                    "constitution": "string",
                    "intelligence": "string",
                    "wisdom": "string",
                    "charisma": "string"
                }
            }
        },
        "immunity": {
            "type": "array",
            "items": "string"
        },
        "required": [
            "name",
            "level",
            "proficiency",
            "max_hp",
            "armour_class",
            "resources",
            "actions",
            "stats",
            "savethrows"
        ]
    }
)";
