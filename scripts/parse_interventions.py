import re
import json
import os


def clean_text(t: str) -> str:
    return re.sub(r'\s+', ' ', t).strip()


SHORT_CATEGORY_OVERRIDES = {
    "ADHD, OCD & Executive Functioning": "Executive & OCD Skills",
    "Affect Identification and Emotional Processing": "Affect & Emotion Processing",
    "Anxiety And Stress Management": "Anxiety & Stress",
    "Anxiety, Depression, And Interpersonal Concerns": "Anxiety, Mood & Relational",
    "Anxiety, Uncertainty & Existential Concerns": "Existential Anxiety",
    "Assessment, Engagement, and Treatment Structure": "Assessment & Engagement",
    "Attachment Dynamics in Couples Relationships": "Couples Attachment Dynamics",
    "Attachment History & Internal Working Models": "Attachment & Working Models",
    "Attachment Injuries, Betrayal & Relational Ruptures": "Attachment Injury & Repair",
    "Attachment Injuries, Loss & Unresolved Relational Experiences": "Attachment Loss & Repair",
    "Attachment Needs, Fears & Emotional Vulnerability": "Attachment Needs & Fears",
    "Attachment Needs, Fears & Longing": "Attachment Needs & Longing",
    "Attachment Patterns & Relational Security": "Attachment & Security",
    "Attachment Styles & Internal Working Models": "Attachment Styles & Models",
    "Attachment, Relationships & Interpersonal Compassion": "Relational Compassion",
    "Autonomy, Personal Agency, and Client-Led Growth": "Autonomy & Personal Agency",
    "Avoidance, Safety Behaviors & Behavioral Flexibility": "Avoidance & Flexibility",
    "Behavioral Chain Analysis and Dialectical Strategies": "Chain Analysis & Dialectics",
    "Behavioral Change And Problem-Solving": "Behavioral Problem-Solving",
    "Behavioral Change and Relationship Enhancement": "Relationship Enhancement",
    "Between-Session Practice and Progress Monitoring": "Practice & Progress Review",
    "Body Awareness & Interoceptive Awareness": "Interoceptive Body Awareness",
    "Boundaries, Autonomy & Differentiation": "Boundaries & Autonomy",
    "Boundaries, Differentiation, and Emotional Regulation": "Boundaries & Differentiation",
    "CPT Psychoeducation and Trauma Processing": "CPT Trauma Psychoeducation",
    "CPT Worksheets and Structured Cognitive Exercises": "CPT Cognitive Worksheets",
    "Change, Growth & Personal Transformation": "Growth & Transformation",
    "Clarification, Interpretation, and Self-Understanding": "Clarification & Insight",
    "Clarifying Relationship Options and Future Direction": "Relationship Options",
    "Cognitive Interventions & Inhibitory Learning": "Inhibitory Learning",
    "Cognitive Restructuring & Beliefs": "Cognitive Restructuring",
    "Cognitive Restructuring and Relationship Beliefs": "Relationship Beliefs",
    "Committed Action And Behavioral Change": "Committed Action",
    "Communication & Conflict Management": "Communication & Conflict",
    "Communication & Interpersonal Effectiveness": "Interpersonal Effectiveness",
    "Communication and Behavioral Skills": "Communication Skills",
    "Communication, Conflict, and Problem-Solving": "Conflict & Problem-Solving",
    "Communication, Emotional Expression & Couples Engagement": "Couples Emotional Expression",
    "Compassion Practices, Imagery & Experiential Exercises": "Compassion Imagery",
    "Compassionate Action, Integration & Maintenance": "Compassionate Action",
    "Compassionate Imagery & the Compassionate Self": "The Compassionate Self",
    "Conflict Resolution and Problem-Solving": "Conflict Resolution",
    "Conflict, Communication & Self-Advocacy": "Conflict & Self-Advocacy",
    "Conflict, Disagreements & Relational Repair": "Conflict & Relational Repair",
    "Coping Questions & Resilience": "Coping & Resilience",
    "Couples: Shared Meaning, Commitment & Relational Choices": "Shared Meaning & Commitment",
    "Decision Consolidation and Transition Planning": "Decision & Transition Plan",
    "Defense Mechanisms and Resistance": "Defenses & Resistance",
    "Depression, Anxiety & Symptom Management": "Mood & Symptom Management",
    "Developmental History and Attachment": "Developmental Attachment",
    "Differing Values, Expectations & Life Goals": "Values & Life Goals",
    "Dissociation, Disconnection & Reconnection": "Dissociation & Reconnection",
    "Eliciting and Strengthening Change Talk": "Eliciting Change Talk",
    "Emotional Awareness And Regulation": "Emotion Regulation",
    "Emotional Awareness and Self-Understanding": "Emotional Self-Understanding",
    "Emotional Awareness, Acceptance & Regulation": "Emotional Acceptance",
    "Emotional Connection, Intimacy & Attachment": "Connection & Intimacy",
    "Emotional Intimacy, Vulnerability & Connection": "Intimacy & Vulnerability",
    "Emotional Needs, Validation & Interpersonal Awareness": "Validation & Needs",
    "Emotional Processing Through the Body": "Somatic Emotion Processing",
    "Emotional Regulation & Attachment Activation": "Attachment Regulation",
    "Emotional Regulation & Distress Tolerance": "Regulation & Tolerance",
    "Emotional Regulation, Trust, and Relationship Maintenance": "Regulation & Trust",
    "Engaging & Foundational Spirit": "MI Foundational Spirit",
    "Exception-Finding Interventions": "Exception-Finding",
    "Examining Each Partner's Contribution to Relationship Difficulties": "Partner Contributions",
    "Existential Anxiety, Uncertainty & Mortality": "Uncertainty & Mortality",
    "Experiential Exercises And Treatment Maintenance": "Experiential & Maintenance",
    "Exploring Ambivalence and Discrepancies": "Ambivalence & Discrepancy",
    "Exploring Each Partner's Perspective": "Partner Perspectives",
    "Exposure Hierarchy & Treatment Planning": "Exposure Hierarchy",
    "Exposure Review, Monitoring & Between-Session Practice": "Exposure Review & Practice",
    "Family Dynamics & Intergenerational Patterns": "Intergenerational Patterns",
    "Family Dynamics, Roles & Relationship Development": "Family Roles & Dynamics",
    "Family-of-Origin Influences and Intergenerational Patterns": "Family-of-Origin Patterns",
    "Feedback, Reinforcement & Maintenance": "Feedback & Maintenance",
    "Freedom, Choice & Personal Responsibility": "Choice & Responsibility",
    "Friendship, Connection & Intimacy": "Friendship & Intimacy",
    "Goal Setting & Action Planning": "Goal & Action Planning",
    "Grief, Loss & Emotional Processing": "Grief & Emotional Loss",
    "Grounding & Present-Moment Awareness": "Grounding & Presence",
    "Identity, Authenticity & Self-Understanding": "Identity & Authenticity",
    "Imaginal Exposure & Intrusive Thoughts": "Imaginal Exposure",
    "Impulse Management and Skills Generalization": "Impulse & Skill Transfer",
    "In Vivo Exposure Interventions": "In Vivo Exposure",
    "Insight Development and Exploration": "Insight & Exploration",
    "Insight Integration and Treatment Planning": "Insight Integration",
    "Integration, Recovery, and Maintenance": "Recovery & Maintenance",
    "Integration, Reflection & Intentional Living": "Intentional Living",
    "Interpersonal & Couples Interventions": "Interpersonal & Couples",
    "Interpersonal And Relationship-Focused Cbt": "Interpersonal CBT",
    "Interpersonal Effectiveness Skills": "Interpersonal Skills",
    "Life Changes, Transitions & Adjustment": "Transitions & Adjustment",
    "Life Goals, Decisions & Personal Development": "Life Goals & Decisions",
    "Maintenance, Relapse Prevention & Treatment Progress": "Relapse Prevention",
    "Meaning, Purpose & Personal Fulfillment": "Meaning & Fulfillment",
    "Mortality, Loss & Life Transitions": "Loss & Life Transitions",
    "Motivation, Ambivalence & Change Readiness": "Change Readiness",
    "Movement, Physical Expression & Somatic Release": "Movement & Somatic Release",
    "Negative Interactional Cycles (Couples)": "Negative Couple Cycles",
    "Nervous System Regulation & Arousal Management": "Nervous System Regulation",
    "Perfectionism, Mistakes & Performance Pressure": "Perfectionism & Pressure",
    "Planning, Commitment, and Maintenance of Change": "Change Planning",
    "Present-Moment Awareness And Mindfulness": "Mindful Presence",
    "Psychoeducation & OCD Cycle Identification": "OCD Cycle Psychoeducation",
    "Psychoeducation And Treatment Maintenance": "Psychoeducation & Care",
    "Psychoeducation, ERP Rationale & Hierarchy Development": "ERP Rationale & Hierarchy",
    "Psychological Flexibility And Avoidance": "Psychological Flexibility",
    "Reframing, Perspective & Meaning": "Reframing & Meaning",
    "Relational Accessibility, Responsiveness & Engagement (Couples)": "Accessibility & Engagement",
    "Relational Dynamics and Therapeutic Relationship": "Therapeutic Relationship",
    "Relational Exploration & Relationship Dynamics": "Relational Dynamics",
    "Relational Growth, Change & Maintenance": "Relational Growth",
    "Relational Meaning, Patterns & Connection": "Relational Meaning",
    "Relational Patterns & Interpersonal Functioning": "Interpersonal Patterns",
    "Relationship & Interpersonal Issues": "Interpersonal Issues",
    "Relationship Assessment & Patterns": "Relationship Assessment",
    "Relationship Decision-Making & Relational Expectations": "Relational Decisions",
    "Relationships, Boundaries, and Therapeutic Change": "Boundaries & Change",
    "Relationships, Connection & Existential Isolation": "Connection & Isolation",
    "Response Prevention & Compulsive Behavior Reduction": "Response Prevention",
    "Scaling Questions & Progress Monitoring": "Scaling & Progress",
    "Secure Attachment Development & Integration": "Secure Attachment",
    "Self-Concept, Congruence, and Self-Acceptance": "Congruence & Acceptance",
    "Self-Concept, Shame & Emotional Regulation (Individual and Couples)": "Self-Concept & Shame",
    "Self-Criticism & Inner Critic Work": "Inner Critic Work",
    "Self-Criticism, Shame & Threat-Based Patterns": "Shame & Threat Patterns",
    "Self-Esteem, Depression, And Trauma-Related Beliefs": "Self-Esteem & Trauma Beliefs",
    "Self-Esteem, Identity, and Emotional Conflict": "Identity & Conflict",
    "Self-Image, Identity & Meaning": "Identity & Self-Image",
    "Solution-Building & the Miracle Question": "Miracle Question & Solutions",
    "Somatic Boundaries, Agency & Empowerment": "Somatic Boundaries & Agency",
    "Somatic Integration, Self-Regulation & Maintenance": "Somatic Self-Regulation",
    "Somatic Interventions for Couples & Relational Distress": "Couples Somatic Regulation",
    "Soothing Rhythm Breathing & Physiological Regulation": "Soothing Rhythm Breathing",
    "Strengthening Self-Efficacy and Addressing Barriers": "Self-Efficacy & Barriers",
    "Strengths, Resources & Client Competence": "Strengths & Resources",
    "Stuck Point Identification and Cognitive Restructuring": "Stuck Point Restructuring",
    "Systemic & Interactional Patterns": "Systemic Patterns",
    "The Five CPT Cognitive Themes": "Five CPT Themes",
    "Trauma Assessment, Psychoeducation, and Stabilization": "Trauma Stabilization",
    "Trauma Narrative and Impact Statement Work": "Trauma Impact Statement",
    "Trauma Processing and Emotional Exploration": "Trauma Emotion Processing",
    "Trauma, Childhood & Attachment Experiences": "Childhood & Trauma",
    "Trauma, Vulnerability & Protective Responses": "Protective Trauma Responses",
    "Trauma-Informed Somatic Interventions": "Trauma-Informed Somatic",
    "Trauma-Related Behaviors, Avoidance, and Regulation": "Trauma Avoidance & Coping",
    "Trauma-Related Cognitions and Belief Restructuring": "Trauma Belief Restructuring",
    "Trauma-Related Guilt, Shame, and Self-Blame": "Trauma Guilt & Shame",
    "Triangles, Family Structure, Roles, and Systemic Change": "Triangles & Family Roles",
    "Trust, Boundaries & Relationship Challenges": "Trust & Boundaries",
    "Trust, Emotional Safety & Relational Security": "Trust & Emotional Safety",
    "Uncertainty Tolerance & Distress Management": "Uncertainty Tolerance",
    "Values, Beliefs & Life Priorities": "Values & Priorities",
    "Vulnerability, Self-Expression & Enactments": "Vulnerability & Enactments",
    "Work, Lifestyle & Daily Functioning": "Work & Daily Balance",
}


def normalize_category(cat: str) -> str:
    c = clean_text(cat)
    if c in SHORT_CATEGORY_OVERRIDES:
        return SHORT_CATEGORY_OVERRIDES[c]
    for k, v in SHORT_CATEGORY_OVERRIDES.items():
        if k.lower() == c.lower():
            return v
    return c


def build_data():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    file_path_2 = os.path.join(base_dir, 'email_2_interventions_modalities.md')
    file_path_3 = os.path.join(base_dir, 'email_3_body.txt')

    with open(file_path_2, 'r', encoding='utf-8') as f:
        text2 = f.read().replace('\r\n', '\n').replace('\r', '\n')

    modality_splits_2 = [
        ('Cognitive Behavioral Therapy (CBT)', 'CBT', r'CBT:'),
        ('Acceptance and Commitment Therapy (ACT)', 'ACT', r'ACT:'),
        ('Psychodynamic Therapy', 'Psychodynamic', r'Psychodynamic:'),
        ('Dialectical Behavior Therapy (DBT)', 'DBT', r'DBT:'),
        ('Cognitive Processing Therapy (CPT)', 'CPT', r'Cognitive Processing Therapy \(CPT\)'),
        ('CBT for Couples', 'CBT Couples', r'CBT FOR COUPLES:'),
        ('Discernment Counseling', 'Discernment', r'Discernment counseling for therapists:'),
        ('Trauma-Focused Therapy', 'Trauma-Focused', r'Trauma focused:'),
        ('Person-Centered Therapy', 'Person-Centered', r'Person-\s*Centered therapy:'),
        ('Motivational Interviewing', 'Motivational', r'\*Motivational Interviewing:\s*\*?'),
        ('Gottman Method', 'Gottman', r'\*Gottman Method:\*?'),
        ('Family Systems Therapy', 'Family Systems', r'F\*?amily Systems:\*?')
    ]

    default_first_cats = {
        'CBT': 'Cognitive Restructuring',
        'ACT': 'Cognitive Defusion',
        'Person-Centered': 'Core Therapeutic Conditions',
        'Motivational': 'MI Foundational Spirit',
        'Gottman': 'Relationship Assessment',
        'Family Systems': 'Systemic Patterns'
    }

    positions = []
    for full_name, short_code, pattern in modality_splits_2:
        m = re.search(pattern, text2, re.IGNORECASE)
        if m:
            positions.append((m.start(), full_name, short_code))
    positions.sort()

    all_interventions = []
    seen_titles = set()

    for idx in range(len(positions)):
        start_pos, full_name, short_code = positions[idx]
        end_pos = positions[idx + 1][0] if idx + 1 < len(positions) else len(text2)
        mod_text = text2[start_pos:end_pos]
        mod_text = re.split(r'\*Liz Wegner|This email is intended', mod_text)[0]

        mod_items = []

        if short_code in ('CBT', 'ACT'):
            current_cat = default_first_cats[short_code]
            lines = [l.strip() for l in mod_text.split('\n') if l.strip()]
            expected_num = 1
            i = 1
            while i < len(lines):
                line = lines[i]
                m_item = re.match(rf'^{expected_num}\.\s+(.+)$', line)
                if m_item:
                    title = clean_text(m_item.group(1))
                    desc_lines = []
                    i += 1
                    while i < len(lines):
                        nxt = lines[i]
                        if re.match(rf'^{expected_num + 1}\.\s+', nxt):
                            break
                        if (re.match(r'^[2-9]\.\s+[A-Z]', nxt)
                                and i + 1 < len(lines)
                                and re.match(rf'^{expected_num + 1}\.\s+', lines[i + 1])):
                            break
                        desc_lines.append(nxt)
                        i += 1
                    mod_items.append((title, clean_text(' '.join(desc_lines)), normalize_category(current_cat)))
                    expected_num += 1
                else:
                    m_cat = re.match(r'^[2-9]\.\s+(.+)$', line)
                    if m_cat:
                        current_cat = normalize_category(m_cat.group(1).title())
                    i += 1
        elif short_code in ('Psychodynamic', 'DBT'):
            current_cat = 'Core Interventions'
            lines = [l.strip() for l in mod_text.split('\n') if l.strip()][1:]
            i = 0
            while i < len(lines):
                line = lines[i]
                if ':' not in line and not line.endswith('.'):
                    current_cat = normalize_category(line)
                    i += 1
                    continue
                m_item = re.match(r'^([A-Z][A-Za-z0-9 &,\'\(\)\-]+?):\s*(.+)$', line)
                if m_item:
                    title = clean_text(m_item.group(1))
                    desc_parts = [m_item.group(2)]
                    i += 1
                    while i < len(lines) and not desc_parts[-1].endswith('.'):
                        desc_parts.append(lines[i])
                        i += 1
                    mod_items.append((title, clean_text(' '.join(desc_parts)), current_cat))
                else:
                    i += 1
        elif short_code == 'Gottman':
            flat = clean_text(mod_text)
            flat = re.sub(r'^\*Gottman Method:\*\s*\*\s*', '', flat)
            flat = re.sub(r'\s*\*\s*$', '', flat)
            gottman_cats = {
                1: 'Relationship Assessment',
                11: 'Communication & Conflict',
                21: 'Friendship & Intimacy',
                31: 'Trust, Commitment & Repair',
                41: 'Shared Meaning & Values'
            }
            current_cat = gottman_cats[1]
            for n in range(1, 51):
                if n in gottman_cats:
                    current_cat = gottman_cats[n]
                if n < 50:
                    pat = rf'(?:^|\s){n}\.\s+([^:]+):\s*(.+?)(?=\s+(?:[2-5]\.\s+[A-Z][^0-9]+?\s+)?{n+1}\.\s+)'
                else:
                    pat = rf'(?:^|\s)50\.\s+([^:]+):\s*(.+)$'
                m_g = re.search(pat, flat)
                if m_g:
                    mod_items.append((clean_text(m_g.group(1)), clean_text(m_g.group(2)), current_cat))
        else:
            current_cat = default_first_cats.get(short_code, 'Core Interventions')
            lines = [l.strip() for l in mod_text.split('\n') if l.strip()][1:]
            expected_num = 1
            i = 0
            while i < len(lines):
                line = lines[i]
                m_item = re.match(rf'^{expected_num}\.\s+([^:]+):\s*(.+)$', line)
                if m_item:
                    title = clean_text(m_item.group(1))
                    desc_parts = [m_item.group(2)]
                    i += 1
                    while i < len(lines) and not desc_parts[-1].endswith('.'):
                        desc_parts.append(lines[i])
                        i += 1
                    mod_items.append((title, clean_text(' '.join(desc_parts)), normalize_category(current_cat)))
                    expected_num += 1
                else:
                    m_cat = re.match(r'^[1-9]\.\s+([^:]+)$', line)
                    if m_cat and not line.endswith('.'):
                        current_cat = normalize_category(m_cat.group(1))
                    i += 1

        for idx_item, (t, d, c) in enumerate(mod_items, 1):
            key = (short_code, t.lower())
            seen_titles.add(key)
            all_interventions.append({
                'id': f'{short_code.lower().replace(" ", "_")}_{idx_item}',
                'modality': full_name,
                'shortModality': short_code,
                'category': c,
                'title': t,
                'description': d,
                'fullText': f'{t}: {d}'
            })

    extra_modalities = []
    email3_session_items = []
    if os.path.exists(file_path_3):
        with open(file_path_3, 'r', encoding='utf-8', errors='ignore') as f:
            text3 = f.read().replace('\r\n', '\n').replace('\r', '\n')

        sf_marker = 'Master List: 150 Generalized Therapy Session Focus Topics'
        sf_pos = text3.find(sf_marker)
        int_part = text3[:sf_pos] if sf_pos != -1 else text3
        sf_part = text3[sf_pos:] if sf_pos != -1 else ''

        email3_sections = [
            ('Solution-Focused Brief Therapy (SFBT)', 'SFBT', 'Top 100 Solution-Focused Brief Therapy (SFBT) Interventions'),
            ('Compassion-Focused Therapy (CFT)', 'CFT', 'Top 100 Compassion-Focused Therapy (CFT) Interventions'),
            ('Exposure and Response Prevention (ERP)', 'ERP', 'Top 100 Exposure and Response Prevention (ERP) Interventions'),
            ('Relational Therapy', 'Relational', 'Top 100 Relational Therapy Interventions'),
            ('Emotionally Focused Therapy (EFT)', 'EFT', 'Top 100 Emotionally Focused Therapy (EFT) Interventions'),
            ('Attachment-Based Therapy', 'Attachment', 'Top 100 Attachment-Based Therapy Interventions'),
            ('Existential Therapy', 'Existential', 'Top 100 Existential Therapy Interventions'),
            ('Somatic Therapy', 'Somatic', 'Top 100 Somatic Therapy Interventions'),
        ]

        sec_positions = []
        for m_info in re.finditer(r'Top 100 [^\n]+', int_part):
            sec_positions.append((m_info.start(), m_info.group(0)))
        sec_positions.sort()

        for full_name, short_code, header_prefix in email3_sections:
            extra_modalities.append(full_name)
            start_idx = int_part.find(header_prefix)
            if start_idx == -1:
                continue
            end_idx = len(int_part)
            for pos, _ in sec_positions:
                if pos > start_idx:
                    end_idx = pos
                    break
            chunk = int_part[start_idx:end_idx]

            cat_headers = []
            for m_c in re.finditer(
                r'(?:^|\n|Interventions)\s*([1-9]|10)\.\s*\n*\s*([A-Z][^—\n]+(?:\n\s*[A-Z][A-Za-z]+\s*)?)(?=\n\s*\n\s*\d+\.|\n\s*\d+\.)',
                chunk
            ):
                cat_headers.append((m_c.start(), normalize_category(m_c.group(2))))

            item_matches = re.finditer(
                r'\n\s*(\d+)\.\s*\n*\s*[A-Za-z0-9 &/\-\(\)\',]+?:\s*([^—]+?)\s*—\s*(.+?)(?=\n\s*\d+\.\s|\n\s*(?:[1-9]|10)\.\s+[A-Z]|\Z)',
                chunk,
                re.DOTALL
            )
            count_for_mod = 0
            for m_i in item_matches:
                pos = m_i.start()
                cat = 'Core Interventions'
                for cpos, cname in cat_headers:
                    if cpos < pos:
                        cat = cname
                t = clean_text(m_i.group(2))
                d = clean_text(m_i.group(3))
                if not d.endswith('.'):
                    if d.endswith('emotional needs and fears'):
                        d += ' from reactive interpersonal patterns.'
                    else:
                        d += '.'
                key = (short_code, t.lower())
                if key in seen_titles or len(t) > 85 or len(d) < 10:
                    continue
                seen_titles.add(key)
                count_for_mod += 1
                all_interventions.append({
                    'id': f'{short_code.lower()}_{count_for_mod}',
                    'modality': full_name,
                    'shortModality': short_code,
                    'category': cat,
                    'title': t,
                    'description': d,
                    'fullText': f'{t}: {d}'
                })

        if sf_part:
            current_sf_cat = 'General Clinical Themes'
            cat_headers = list(re.finditer(r'(?:^|\n|Statements)([A-O])\.\s+([A-Za-z0-9 &,\-/]+)', sf_part))
            sf_matches = list(re.finditer(
                r'\n\s*(\d+)\.\s*\n+\s*([^:\n]+):\s*(.+?)(?=\n\s*\d+\.\s*\n|\n[A-O]\.\s+|\nPart 2:|\n\*Liz|\Z)',
                sf_part,
                re.DOTALL
            ))
            for m in sf_matches:
                pos = m.start()
                for ch in cat_headers:
                    if ch.start() < pos:
                        current_sf_cat = normalize_category(ch.group(2))
                num = int(m.group(1))
                t = clean_text(m.group(2))
                d = clean_text(m.group(3))
                email3_session_items.append({
                    'id': f'sf_m_{num}',
                    'title': t,
                    'description': d,
                    'category': current_sf_cat
                })

    session_focus_items = [
        {
            'id': 'sf_1',
            'title': 'Anxiety Symptoms & Worry Patterns',
            'description': 'Explored escalating worry, somatic tension, and avoidance behaviors across daily environments.',
            'category': 'Anxiety & Stress'
        },
        {
            'id': 'sf_2',
            'title': 'Depressive Symptoms & Low Motivation',
            'description': 'Addressed persistent feelings of hopelessness, fatigue, withdrawal, and diminished interest in pleasurable activities.',
            'category': 'Mood & Affect'
        },
        {
            'id': 'sf_3',
            'title': 'Interpersonal Communication & Boundary Setting',
            'description': 'Focused on recurrent conflict, passive-aggressive communication, and difficulties maintaining clear personal boundaries with family or partner.',
            'category': 'Relationships'
        },
        {
            'id': 'sf_4',
            'title': 'Trauma Processing & Flashbacks',
            'description': 'Examined recent trauma triggers, somatic hyperarousal, intrusive thoughts, and emotional numbing.',
            'category': 'Trauma'
        },
        {
            'id': 'sf_5',
            'title': 'Life Transitions & Career Stress',
            'description': 'Evaluated psychological stressors associated with major role shifts, workplace overwhelm, and identity restructuring.',
            'category': 'Life Transitions'
        },
        {
            'id': 'sf_6',
            'title': 'Grief, Bereavement & Loss',
            'description': 'Processed acute emotional pain, waves of sadness, and adaptive adjustments following significant relational loss.',
            'category': 'Grief & Loss'
        },
        {
            'id': 'sf_7',
            'title': 'Emotional Dysregulation & Anger Surges',
            'description': 'Examined situational triggers leading to impulsive anger, behavioral outbursts, and subsequent guilt or regret.',
            'category': 'Emotion Regulation'
        },
        {
            'id': 'sf_8',
            'title': 'Negative Self-Appraisal & Shame',
            'description': 'Identified persistent core beliefs of inadequacy, chronic self-criticism, and impostor fears.',
            'category': 'Self-Concept'
        },
        {
            'id': 'sf_9',
            'title': 'Substance Craving & Behavioral Impulses',
            'description': 'Assessed environmental cues, coping deficits, and high-risk relapse contexts affecting sobriety.',
            'category': 'Addiction & Impulses'
        },
        {
            'id': 'sf_10',
            'title': 'Marital Disconnection & Repair Attempts',
            'description': 'Explored emotional distance, mutual criticism, and missed bids for intimacy between partners.',
            'category': 'Couples & Family'
        }
    ] + email3_session_items

    risk_factors_items = [
        {
            'id': 'rf_7',
            'title': 'No Acute Risk Factors Identified',
            'description': 'Client denied suicidal or homicidal ideation, intent, or plan. Denied NSSI, psychotic symptoms, or immediate safety vulnerabilities.',
            'category': 'Routine Screening',
            'severity': 'none'
        },
        {
            'id': 'rf_1',
            'title': 'Non-Suicidal Self-Injury (NSSI) - Historical / Non-Acute',
            'description': 'Client reports past history of NSSI; no urges or behaviors present within the past 30 days. Coping alternatives affirmed.',
            'category': 'Self-Harm',
            'severity': 'mild'
        },
        {
            'id': 'rf_2',
            'title': 'Passive Suicidal Ideation - No Intent or Plan',
            'description': 'Client endorsed intermittent passive thoughts of wanting relief; denies intent, plan, or access to lethal means. Safety plan reviewed.',
            'category': 'Suicidality',
            'severity': 'moderate'
        },
        {
            'id': 'rf_3',
            'title': 'Active Suicidal Ideation - Safety Protocol Executed',
            'description': 'Client expressed active thoughts with ambivalence. Comprehensive safety planning completed; emergency contacts and 988 crisis line reinforced.',
            'category': 'Suicidality',
            'severity': 'elevated'
        },
        {
            'id': 'rf_4',
            'title': 'Substance Misuse / High-Risk Coping',
            'description': 'Client noted increased alcohol or sedative use in response to emotional distress. Relapse prevention strategies and harm reduction reviewed.',
            'category': 'Substance Use',
            'severity': 'moderate'
        },
        {
            'id': 'rf_5',
            'title': 'Intimate Partner Violence (IPV) - Screened & Monitored',
            'description': 'Screened for emotional, physical, and psychological safety within relationship. Confidential safety resources provided.',
            'category': 'Interpersonal Safety',
            'severity': 'elevated'
        },
        {
            'id': 'rf_6',
            'title': 'Severe Panic & Dissociative Episodes',
            'description': 'Episodes of derealization and hyperventilation experienced under stress. Grounding and 5-4-3-2-1 somatic strategies reinforced.',
            'category': 'Somatic & Trauma',
            'severity': 'mild'
        },
        {
            'id': 'rf_8',
            'title': 'Acute Sleep Deprivation & Circadian Disruption',
            'description': 'Client reported significant insomnia and fragmented sleep impacting daytime emotional regulation. Sleep hygiene and medical follow-up discussed.',
            'category': 'Physiological Risk',
            'severity': 'mild'
        },
        {
            'id': 'rf_9',
            'title': 'Disordered Eating / Restrictive or Binge Urges',
            'description': 'Monitored nutritional restriction and compensatory urges triggered by body-image distress. Multidisciplinary medical oversight affirmed.',
            'category': 'Eating & Somatic',
            'severity': 'moderate'
        },
        {
            'id': 'rf_10',
            'title': 'Caregiver Burnout & Acute Environmental Exhaustion',
            'description': 'Client experiencing chronic role overload and secondary stress without adequate respite support. Protective boundaries and crisis supports reviewed.',
            'category': 'Psychosocial Stressors',
            'severity': 'mild'
        },
        {
            'id': 'rf_11',
            'title': 'Financial / Housing Instability Stressors',
            'description': 'Assessed acute socioeconomic stressors affecting baseline safety and treatment adherence; community resource navigation provided.',
            'category': 'Social Determinants',
            'severity': 'moderate'
        },
        {
            'id': 'rf_12',
            'title': 'Medication Non-Adherence / Side-Effect Distress',
            'description': 'Client reported skipped doses of prescribed psychotropic medication; encouraged prompt communication with prescribing clinician.',
            'category': 'Medical & Pharmacotherapy',
            'severity': 'moderate'
        }
    ]

    additional_info_items = [
        {
            'id': 'ai_9',
            'title': 'Scheduling: Recurring Weekly Session Confirmed',
            'description': 'Next individual psychotherapy session scheduled for next week at the regular appointment time.',
            'category': 'Scheduling'
        },
        {
            'id': 'ai_1',
            'title': 'Homework Assigned: Thought Record & Distortions Log',
            'description': 'Client agreed to complete a 3-column thought record capturing automatic thoughts and balanced alternatives before next session.',
            'category': 'Homework'
        },
        {
            'id': 'ai_2',
            'title': 'Homework Assigned: Daily Behavioral Activation',
            'description': 'Client scheduled 2 designated pleasurable or mastery-oriented activities per day using the weekly activity log.',
            'category': 'Homework'
        },
        {
            'id': 'ai_3',
            'title': 'Homework Assigned: Mindfulness & STOP Skill Practice',
            'description': 'Rehearsed STOP distress tolerance skill to be practiced once daily and during rising agitation.',
            'category': 'Homework'
        },
        {
            'id': 'ai_4',
            'title': 'Referral Provided: Psychiatric Consultation',
            'description': 'Provided contact information for outpatient psychiatric evaluation for medication management assessment.',
            'category': 'Referrals'
        },
        {
            'id': 'ai_5',
            'title': 'Referral Provided: Support Group / Community Resources',
            'description': 'Provided referrals for peer grief support group and local community mental health resources.',
            'category': 'Referrals'
        },
        {
            'id': 'ai_6',
            'title': 'Coordination of Care: PCP / Collateral Contact',
            'description': 'Client signed ROI for coordination of care with primary care provider regarding somatic symptoms.',
            'category': 'Care Coordination'
        },
        {
            'id': 'ai_7',
            'title': 'Goal for Next Session: Review Exposure Hierarchy',
            'description': 'Next session will review completed exposure hierarchy and initiate in-session imaginal exposure.',
            'category': 'Next Session Goals'
        },
        {
            'id': 'ai_8',
            'title': 'Goal for Next Session: Couple Communication Rehearsal',
            'description': 'Follow up on between-session Speaker-Listener exercise and practice softened startup during conflict.',
            'category': 'Next Session Goals'
        },
        {
            'id': 'ai_10',
            'title': 'Homework Assigned: Values-Based Committed Action Step',
            'description': 'Client identified one concrete, values-aligned behavioral step to execute prior to next session.',
            'category': 'Homework'
        },
        {
            'id': 'ai_11',
            'title': 'Homework Assigned: Somatic Grounding & Breath Pacing',
            'description': 'Client will practice 5 minutes of physiological sigh / paced respiration daily to reinforce autonomic baseline.',
            'category': 'Homework'
        },
        {
            'id': 'ai_12',
            'title': 'Scheduling: Bi-Weekly Maintenance Cadence Confirmed',
            'description': 'Client transitioning to bi-weekly session cadence to consolidate treatment gains and practice independent coping.',
            'category': 'Scheduling'
        }
    ]

    all_modalities = [m[0] for m in modality_splits_2] + extra_modalities

    output_data = {
        'metadata': {
            'totalInterventions': len(all_interventions),
            'modalitiesCount': len(all_modalities),
            'modalities': all_modalities,
            'version': '2.0.0',
            'author': 'Liz Wegner Proprietary Software Collective - Liz Wegner, LLMSW'
        },
        'interventions': all_interventions,
        'sessionFocus': session_focus_items,
        'riskFactors': risk_factors_items,
        'additionalInfo': additional_info_items
    }

    out_dir = os.path.join(base_dir, 'data')
    os.makedirs(out_dir, exist_ok=True)
    out_file = os.path.join(out_dir, 'clinical_banks.json')
    with open(out_file, 'w', encoding='utf-8') as f:
        json.dump(output_data, f, ensure_ascii=False, separators=(',', ':'))

    print('Successfully built clinical banks JSON:')
    print(f'  - Total Interventions: {len(all_interventions)} across {len(all_modalities)} modalities')
    print(f'  - Session Topics items: {len(session_focus_items)}')
    print(f'  - Risk Factors items: {len(risk_factors_items)}')
    print(f'  - Additional Info items: {len(additional_info_items)}')
    print(f'  - File size: {os.path.getsize(out_file)} bytes')


if __name__ == '__main__':
    build_data()
