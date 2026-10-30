import re
import json
import os

def clean_text(t):
    return re.sub(r'\s+', ' ', t).strip()

def build_data():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    file_path = os.path.join(base_dir, 'email_2_interventions_modalities.md')
    with open(file_path, 'r', encoding='utf-8') as f:
        text = f.read()

    modality_splits = [
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

    positions = []
    for full_name, short_code, pattern in modality_splits:
        m = re.search(pattern, text, re.IGNORECASE)
        if m:
            positions.append((m.start(), full_name, short_code))
    positions.sort()

    all_interventions = []

    for idx in range(len(positions)):
        start_pos, full_name, short_code = positions[idx]
        end_pos = positions[idx+1][0] if idx + 1 < len(positions) else len(text)
        mod_text = text[start_pos:end_pos]
        
        mod_items = []
        
        if short_code in ['Psychodynamic', 'DBT']:
            current_cat = 'Core Interventions'
            for block in re.split(r'\n\s*\n', mod_text):
                block = block.strip()
                if not block or block.startswith('http') or block.startswith('*Liz'):
                    continue
                if ':' not in block and len(block) < 60 and not block.lower().startswith('this email'):
                    current_cat = block
                    continue
                match = re.match(r'^([A-Z][A-Za-z0-9 /&,\'\(\)\-]+?):\s*([^\n\r]+(?:\n[^\n\r]+)*)', block)
                if match:
                    t = clean_text(match.group(1))
                    d = clean_text(match.group(2))
                    if len(t) < 70 and len(d) > 10 and t not in ['DBT', 'Psychodynamic', 'Protocol distinction']:
                        mod_items.append((t, d, current_cat))
        else:
            current_cat = 'Core Interventions'
            matches = list(re.finditer(r'(?:^|\s|\n)(\d+)\.\s+([A-Za-z0-9 /&,\'\(\)\-]+?)(?::\s*|\n\s*)(.+?)(?=(?:[\s\n]+\d+\.\s+[A-Z]|\n\n[1-9]\.\s+[A-Z]|$))', mod_text, re.DOTALL))
            for m in matches:
                num = m.group(1)
                t = clean_text(m.group(2))
                raw_d = m.group(3).strip()
                raw_d = re.split(r'\*Liz Wegner|This email is intended', raw_d)[0]
                d = clean_text(raw_d)
                if len(t) < 75 and len(d) > 10 and not t.lower().startswith('http'):
                    mod_items.append((t, d, current_cat))

        for idx_item, (t, d, c) in enumerate(mod_items, 1):
            all_interventions.append({
                'id': f'{short_code.lower().replace(" ", "_")}_{idx_item}',
                'modality': full_name,
                'shortModality': short_code,
                'category': c,
                'title': t,
                'description': d,
                'fullText': f'{t}: {d}',
                'keywords': [w.lower() for w in re.findall(r'[a-zA-Z]{3,}', f'{full_name} {c} {t} {d}')]
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
    ]

    risk_factors_items = [
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
            'id': 'rf_7',
            'title': 'No Acute Risk Factors Identified',
            'description': 'Client denied suicidal or homicidal ideation, intent, or plan. Denied NSSI, psychotic symptoms, or immediate safety vulnerabilities.',
            'category': 'Routine Screening',
            'severity': 'none'
        }
    ]

    additional_info_items = [
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
            'id': 'ai_9',
            'title': 'Scheduling: Recurring Weekly Session Confirmed',
            'description': 'Next individual psychotherapy session scheduled for next week at the regular appointment time.',
            'category': 'Scheduling'
        }
    ]

    output_data = {
        'metadata': {
            'totalInterventions': len(all_interventions),
            'modalitiesCount': len(modality_splits),
            'modalities': [m[0] for m in modality_splits],
            'version': '1.0.0',
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
        json.dump(output_data, f, indent=2, ensure_ascii=False)

    print(f'Successfully built clinical banks JSON:')
    print(f'  - Total Interventions: {len(all_interventions)}')
    print(f'  - Session Focus items: {len(session_focus_items)}')
    print(f'  - Risk Factors items: {len(risk_factors_items)}')
    print(f'  - Additional Info items: {len(additional_info_items)}')
    print(f'  - Destination: {out_file}')

if __name__ == '__main__':
    build_data()
